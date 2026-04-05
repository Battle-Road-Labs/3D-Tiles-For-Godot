#include "CesiumGDModelLoader.h"
#include "CesiumGltf/BufferView.h"
#include "error_names.hpp"
#include "missing_functions.hpp"
#include <cstdint>

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/resources/image_texture.h"
#include "scene/resources/surface_tool.h"
#include "scene/3d/mesh_instance_3d.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "scene/resources/shader.h"
#include "scene/resources/shader_material.h"
#endif

#include <CesiumGltfReader/GltfReader.h>
#include "Utils/CesiumGDTextureLoader.h"
#include "CesiumGltf/ExtensionCesiumRTC.h"
#include "CesiumGltf/ExtensionKhrTextureTransform.h"
#include <CesiumGltf/ExtensionKhrMaterialsUnlit.h>
#include "CesiumGeometry/Transforms.h"

#undef OPAQUE

constexpr int32_t RGBA_CHANNEL_COUNT = 4;
constexpr int32_t RGB_CHANNEL_COUNT = 3;

Ref<ArrayMesh> CesiumGDModelLoader::generate_meshes_from_model(const CesiumGltf::Model& model, Error* error)
{	
	std::vector<CesiumGltf::Mesh> gltfMeshes = model.meshes;

	std::unordered_map<int32_t, Ref<StandardMaterial3D>> materialsMap;

	Ref<ArrayMesh> meshInstance = memnew(ArrayMesh);

	Ref<Shader> texture_transform_shader = godot::ResourceLoader::get_singleton()->load("res://Shaders/spatial_texture_rotation.gdshader");
	texture_transform_shader.instantiate();
	if (!texture_transform_shader.is_valid()) {
		texture_transform_shader.instantiate();

		String code = R"(
		shader_type spatial;

		uniform sampler2D albedo_texture : source_color;
		uniform vec2 uv_offset = vec2(0.0);
		uniform vec2 uv_scale = vec2(1.0);
		uniform float uv_rotation = 0.0;

		vec2 rotate_uv(vec2 uv, float angle) {
			float s = sin(angle);
			float c = cos(angle);
			mat2 rot = mat2(c, -s, s, c);
			return rot * uv;
		}

		void fragment() {
			vec2 uv = UV;
			uv *= uv_scale;
			uv = rotate_uv(uv, uv_rotation);
			uv += uv_offset;
			vec4 albedo = texture(albedo_texture, uv);
			ALBEDO = albedo.rgb;
			ALPHA = albedo.a;
		}
		)";

		texture_transform_shader->set_code(code);
	}

	*error = Error::OK;
	for (const CesiumGltf::Mesh& mesh : gltfMeshes) {
		int32_t surfaceIndex = 0;
		for (const CesiumGltf::MeshPrimitive& primitive : mesh.primitives) {

			const CesiumGltf::Model* modelReference = &model;

			// Create the material for the gltf
			Ref<StandardMaterial3D> godotMaterial = memnew(StandardMaterial3D);
			const CesiumGltf::Material& mat = modelReference->materials.at(primitive.material);
			copy_material_properties(mat, godotMaterial, *modelReference);

			// Then copy all the other properties defined in the file
			Vector<Vector3> vertices = get_attribute_from_primitive<Vector3>(primitive, model, "POSITION");

			if (vertices.is_empty()) {
				ERR_PRINT("Mesh did not have a vertex buffer!");
				return meshInstance;
			}

			Vector<Vector3> normals = get_attribute_from_primitive<Vector3>(primitive, model, "NORMAL", [&](Vector3& normal) {
				// We will Invert all normal IF the cull mode is front
				if (mat.doubleSided) {
					normal *= -1.0;
				}
			});

			Vector<Vector2> textureCoords = get_attribute_from_primitive<Vector2>(primitive, model, "TEXCOORD_0", [](Vector2& uv) {
				uv = uv.clamp(Vector2(0, 0), Vector2(1, 1));
			});
			Vector<Vector2> textureCoords1 = get_attribute_from_primitive<Vector2>(primitive, model, "TEXCOORD_1");

			// Try to get Cesium Overlays if the texcoords are not updated
			if (textureCoords.is_empty()) {
				textureCoords = get_attribute_from_primitive<Vector2>(primitive, model, "_CESIUMOVERLAY_0", [](Vector2& uv) {
	        uv = uv.clamp(Vector2(0, 0), Vector2(1, 1));
				});
			}
			if (textureCoords1.is_empty()) {
				textureCoords1 = get_attribute_from_primitive<Vector2>(primitive, model, "_CESIUMOVERLAY_1", [](Vector2& uv) {
    		});
			}

			Vector<int32_t> indexBuffer = get_index_buffer_from_primitive(primitive, model, error);

			// Default index buffer if it is empty
			if (indexBuffer.is_empty()) {
				for (int32_t i = 0; i < vertices.size(); i++) {
					indexBuffer.push_back(i);
				}
			}

			// Required mesh data
			Array arrays;
			// We need to do some extra stuff if we're on the extension
			#if defined(CESIUM_GD_EXT)
			arrays = generate_array_mesh_ext(vertices, indexBuffer, normals, textureCoords, textureCoords1);

			if (normals.is_empty()) {
				godotMaterial->set_shading_mode(BaseMaterial3D::ShadingMode::SHADING_MODE_UNSHADED);
			}
			
			#elif defined(CESIUM_GD_MODULE)
			arrays.resize(ArrayMesh::ARRAY_MAX);
			arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
			arrays[ArrayMesh::ARRAY_INDEX] = indexBuffer;

			if (!normals.is_empty()) {
				arrays[ArrayMesh::ARRAY_NORMAL] = normals;
			}
			else {
				godotMaterial->set_shading_mode(BaseMaterial3D::ShadingMode::SHADING_MODE_UNSHADED);
			}

			if (!textureCoords.is_empty()) {
				arrays[ArrayMesh::ARRAY_TEX_UV] = textureCoords;
			}

			if (!textureCoords1.is_empty()) {
				arrays[ArrayMesh::ARRAY_TEX_UV2] = textureCoords1;
			}
			#endif
			
			Ref<Material> finalMaterial = godotMaterial;
			if (modelReference->hasExtension<CesiumGltf::ExtensionKhrMaterialsUnlit>()) {
				godotMaterial->set_shading_mode(BaseMaterial3D::ShadingMode::SHADING_MODE_UNSHADED);
			}

			if (modelReference->hasExtension<CesiumGltf::ExtensionKhrTextureTransform>()) {
				const CesiumGltf::ExtensionKhrTextureTransform* texture_transform_ext = modelReference->getExtension<CesiumGltf::ExtensionKhrTextureTransform>();
				if (texture_transform_ext) {
					const std::vector<double>& offsetVec = texture_transform_ext->offset;
					const std::vector<double>& scaleVec = texture_transform_ext->scale;
					float rotation_value = texture_transform_ext->rotation;

					Vector3 offsetVector3 = Vector3(0, 0, 0);
					Vector3 scaleVector3 = Vector3(1, 1, 1);

					if (offsetVec.size() == 2) {
						offsetVector3 = Vector3(offsetVec[0], offsetVec[1], 0);
					}

					if (scaleVec.size() == 2) {
						scaleVector3 = Vector3(scaleVec[0], scaleVec[1], 1);
					}

					if (texture_transform_shader.is_valid()) {
						Ref<ShaderMaterial> shaderMat;
						shaderMat.instantiate();
						shaderMat->set_shader(texture_transform_shader);

						Ref<Texture2D> albedo_texture_mat = godotMaterial->get_texture(BaseMaterial3D::TEXTURE_ALBEDO);
						shaderMat->set_shader_parameter("albedo_texture", albedo_texture_mat);

						shaderMat->set_shader_parameter("uv_offset", offsetVector3);
						shaderMat->set_shader_parameter("uv_scale", scaleVector3);
						shaderMat->set_shader_parameter("uv_rotation", rotation_value);
						
						finalMaterial = shaderMat;
					}
					else {

						if (offsetVec.size() == 2) {
							godotMaterial->set_uv1_offset(offsetVector3);
						}


						if (scaleVec.size() == 2) {
							godotMaterial->set_uv1_scale(scaleVector3);
						}
					}
					
				}
			}

			*error = apply_surface_to_mesh(primitive, meshInstance, arrays);
			meshInstance->surface_set_material(surfaceIndex, finalMaterial);
			materialsMap.insert_or_assign(primitive.material, finalMaterial);

			surfaceIndex++;
		}
	}
	return meshInstance;
}

Vector<Vector3> CesiumGDModelLoader::generate_normals(const Vector<Vector3>& vertices, const Vector<int32_t>& indices) {
	Vector<Vector3> normals;
	normals.resize(vertices.size());

	// Initialize all normals to zero
	for (int i = 0; i < normals.size(); i++) {
		normals.write[i] = Vector3(0, 0, 0);
	}

	// Calculate normals for each triangle
	for (int i = 0; i < indices.size(); i += 3) {
		Vector3 v0 = vertices[indices[i]];
		Vector3 v1 = vertices[indices[i + 1]];
		Vector3 v2 = vertices[indices[i + 2]];

		Vector3 normal = (v1 - v0).cross(v2 - v0).normalized();

		// Add the normal to all three vertices
		normals.write[indices[i]] += normal;
		normals.write[indices[i + 1]] += normal;
		normals.write[indices[i + 2]] += normal;
	}

	// Normalize all normals
	for (int i = 0; i < normals.size(); i++) {
		normals.write[i].normalize();
		normals.write[i] = normals[i] * -1;
	}

	return normals;
}

constexpr Mesh::PrimitiveType CesiumGDModelLoader::cesium_to_godot_primitive_mode(int32_t mode)
{
	using CesiumPrimitiveMode = CesiumGltf::MeshPrimitive::Mode;

	switch (mode) {
	case CesiumPrimitiveMode::TRIANGLES:
		// Add vertices to Godot mesh
		return Mesh::PRIMITIVE_TRIANGLES;
	case CesiumPrimitiveMode::LINES:
		return Mesh::PRIMITIVE_LINES;
	case CesiumPrimitiveMode::POINTS:
		return Mesh::PRIMITIVE_POINTS;
	case CesiumPrimitiveMode::LINE_STRIP:
		return Mesh::PRIMITIVE_LINE_STRIP;
	case CesiumPrimitiveMode::TRIANGLE_STRIP:
		return Mesh::PRIMITIVE_TRIANGLE_STRIP;
	default:
		return Mesh::PrimitiveType::PRIMITIVE_TRIANGLES;
	}
}

glm::dmat4x4 CesiumGDModelLoader::apply_rtc_center(const CesiumGltf::Model& gltf, const glm::dmat4x4& rootTransform) {
	const CesiumGltf::ExtensionCesiumRTC* cesiumRTC = gltf.getExtension<CesiumGltf::ExtensionCesiumRTC>();
	if (cesiumRTC == nullptr) {
		return rootTransform;
	}
	const std::vector<double>& rtcCenter = cesiumRTC->center;
	if (rtcCenter.size() != 3) {
		return rootTransform;
	}
	const double x = rtcCenter[0];
	const double y = rtcCenter[1];
	const double z = rtcCenter[2];
	const glm::dmat4x4 rtcTransform(
		glm::dvec4(1.0, 0.0, 0.0, 0.0),
		glm::dvec4(0.0, 1.0, 0.0, 0.0),
		glm::dvec4(0.0, 0.0, 1.0, 0.0),
		glm::dvec4(x, y, z, 1.0)
	);
	return rootTransform * rtcTransform;
}


glm::dmat4x4 CesiumGDModelLoader::apply_gltf_up_axis_transform(const CesiumGltf::Model& model, const glm::dmat4x4& rootTransform) {
	auto gltfUpAxisIt = model.extras.find("gltfUpAxis");
	if (gltfUpAxisIt == model.extras.end()) {
	// The default up-axis of glTF is the Y-axis, and no other
	// up-axis was specified. Transform the Y-axis to the Z-axis,
	// to match the 3D Tiles specification
	return rootTransform * CesiumGeometry::Transforms::Y_UP_TO_Z_UP;
	}
	const CesiumUtility::JsonValue& gltfUpAxis = gltfUpAxisIt->second;
	int gltfUpAxisValue = static_cast<int>(gltfUpAxis.getSafeNumberOrDefault(1));
	if (gltfUpAxisValue == static_cast<int>(CesiumGeometry::Axis::X)) {
		return rootTransform * CesiumGeometry::Transforms::X_UP_TO_Z_UP;
	} else if (gltfUpAxisValue == static_cast<int>(CesiumGeometry::Axis::Y)) {
		return rootTransform * CesiumGeometry::Transforms::Y_UP_TO_Z_UP;
	} else if (gltfUpAxisValue == static_cast<int>(CesiumGeometry::Axis::Z)) {
		// No transform required
	}
	return rootTransform;
}

Vector<int32_t> CesiumGDModelLoader::get_index_buffer_from_primitive(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::Model& model, Error* error)
{
	Vector<int32_t> indices;

	const CesiumGltf::Accessor& indexAccessor = model.accessors[primitive.indices];
	const CesiumGltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
	const CesiumGltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

	const std::byte* indexData = &indexBuffer.cesium.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

	// Handle different index formats (unsigned short, unsigned byte, etc.)
	for (uint32_t i = 0; i < indexAccessor.count; ++i) {
		int32_t index = 0;
		if (indexAccessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT) {
			index = *(reinterpret_cast<const uint16_t*>(indexData + i * sizeof(uint16_t)));
		}
		else if (indexAccessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_INT) {
			index = *(reinterpret_cast<const uint32_t*>(indexData + i * sizeof(uint32_t)));
		}
		else if (indexAccessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_BYTE) {
			index = *(reinterpret_cast<const uint8_t*>(indexData + i * sizeof(uint8_t)));
		}
		indices.push_back(index);
	}

	// Lastly, correct indices too
	while (indices.size() % 3 != 0) {
		indices.push_back(indices.get(indices.size() - 1)); // Duplicate the last index
	}

	return indices;
}


#if defined(CESIUM_GD_EXT)

Array CesiumGDModelLoader::generate_array_mesh_ext(const Vector<Vector3>& vertices, const Vector<int32_t>& indices, const Vector<Vector3>& normals, const Vector<Vector2>& textureCoords, const Vector<Vector2>& textureCoords2) {
	// Define them as packed arrays (TODO: make this switch at creation time to avoid copying data)
	PackedVector3Array packedVert;
	packedVert.resize(vertices.size());
	for (uint32_t i = 0; i < packedVert.size(); i++) {
		packedVert.set(i, vertices.get(i));
	}


	PackedInt32Array packedIndex;
	packedIndex.resize(indices.size());
	for (uint32_t i = 0; i < packedIndex.size(); i++) {
		packedIndex.set(i, indices.get(i));
	}


	PackedVector3Array packedNormals;
	packedNormals.resize(normals.size());
	for (uint32_t i = 0; i < packedNormals.size(); i++) {
		packedNormals.set(i, normals.get(i));
	}

	PackedVector2Array packedTexUvs;
	packedTexUvs.resize(textureCoords.size());
	for(uint32_t i = 0; i < packedTexUvs.size(); i++) {
		packedTexUvs.set(i, textureCoords.get(i));
	}

	PackedVector2Array packedTexUvs2;
	
	packedTexUvs2.resize(textureCoords2.size());
	for(uint32_t i = 0; i < packedTexUvs2.size(); i++) {
		packedTexUvs2.set(i, textureCoords2.get(i));
	}

	Array arrays;

	arrays.resize(ArrayMesh::ARRAY_MAX);
	arrays[ArrayMesh::ARRAY_VERTEX] = packedVert;
	arrays[ArrayMesh::ARRAY_INDEX] = packedIndex;

	if (!normals.is_empty()) {
		arrays[ArrayMesh::ARRAY_NORMAL] = packedNormals;
	}
	
	if (!textureCoords.is_empty()) {
		arrays[ArrayMesh::ARRAY_TEX_UV] = packedTexUvs;
	}

	if (!textureCoords2.is_empty()) {
		arrays[ArrayMesh::ARRAY_TEX_UV2] = packedTexUvs2;
	}
	return arrays;
}
#endif

Error CesiumGDModelLoader::apply_surface_to_mesh(const CesiumGltf::MeshPrimitive& meshPrimitive, Ref<ArrayMesh>& meshInstance, const Array& arrays)
{
	Mesh::PrimitiveType primitiveType = cesium_to_godot_primitive_mode(meshPrimitive.mode);
	meshInstance->add_surface_from_arrays(primitiveType, arrays);
	return Error::OK;
}

Error CesiumGDModelLoader::copy_material_properties(const CesiumGltf::Material& cesiumMaterial, Ref<StandardMaterial3D>& godotMaterial, const CesiumGltf::Model& modelReference)
{
	set_colors_and_texture(cesiumMaterial, godotMaterial, modelReference);

	BaseMaterial3D::CullMode cullMode = cesiumMaterial.doubleSided ? BaseMaterial3D::CULL_DISABLED : BaseMaterial3D::CULL_FRONT;
	BaseMaterial3D::Transparency alphaMode;

	if (cesiumMaterial.alphaMode == CesiumGltf::Material::AlphaMode::OPAQUE) {
		alphaMode = BaseMaterial3D::Transparency::TRANSPARENCY_DISABLED;
	}
	else if (cesiumMaterial.alphaMode == CesiumGltf::Material::AlphaMode::BLEND) {
		alphaMode = BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA;
	}
	else {
		alphaMode = BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA_SCISSOR;
	}

	godotMaterial->set_transparency(alphaMode);
	godotMaterial->set_cull_mode(cullMode);
	godotMaterial->set_name(cesiumMaterial.name.c_str());
	godotMaterial->set_alpha_antialiasing(BaseMaterial3D::ALPHA_ANTIALIASING_ALPHA_TO_COVERAGE);
	godotMaterial->set_shading_mode(BaseMaterial3D::SHADING_MODE_PER_PIXEL);
	godotMaterial->set_flag(BaseMaterial3D::FLAG_USE_TEXTURE_REPEAT, false);
	return Error::OK;
}

void CesiumGDModelLoader::set_colors_and_texture(const CesiumGltf::Material& cesiumMaterial, Ref<StandardMaterial3D>& godotMaterial, const CesiumGltf::Model& modelReference)
{
	if (!cesiumMaterial.pbrMetallicRoughness.has_value()) {
		return;
	}

	const std::vector<double>& baseColorFactor = cesiumMaterial.pbrMetallicRoughness->baseColorFactor;

	if (baseColorFactor.size() >= RGBA_CHANNEL_COUNT) {
		// RGBA constructor
		Color baseColor(baseColorFactor.at(0), baseColorFactor.at(1), baseColorFactor.at(2), baseColorFactor.at(3));
		// GLTF uses linear space, Godot uses sRGB when presenting it to the screen
		baseColor = baseColor.linear_to_srgb();
		godotMaterial->set_albedo(baseColor);
	}

	// TODO: Check if specular or metallic here
	godotMaterial->set_specular(cesiumMaterial.pbrMetallicRoughness->metallicFactor);
	// godotMaterial->set_metallic(cesiumMaterial.pbrMetallicRoughness->metallicFactor);
	godotMaterial->set_roughness(cesiumMaterial.pbrMetallicRoughness->roughnessFactor);

	// TODO: Texture
	const std::optional<CesiumGltf::TextureInfo>& baseTexture = cesiumMaterial.pbrMetallicRoughness->baseColorTexture;
	if (!baseTexture.has_value()) {
		return;
	}
	// Something to get the texture

	const int32_t imageIndex = modelReference.textures.at(baseTexture->index).source;
	const CesiumGltf::Image& image = modelReference.images.at(imageIndex);
	Ref<Texture> textureToUse = CesiumGDTextureLoader::load_image_texture(*image.pAsset.get(), true, false);
	godotMaterial->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, textureToUse);
}

Error CesiumGDModelLoader::generate_normals(Vector<Vector3>* normalBuffer, const Vector<Vector3>& vertexBuffer, const Vector<int32_t>& indexBuffer, bool flip /*= false*/)
{
	ERR_FAIL_COND_V_MSG(!normalBuffer->is_empty(), Error::ERR_INVALID_PARAMETER, "Normal buffer must be empty to generate it!");

	for (int32_t vertex = 0; vertex < vertexBuffer.size(); vertex++) {
		normalBuffer->push_back(Vector3(0, 0, 0));
	}

	constexpr int32_t TRIANGLE_STEPPING = 3;

	for (int32_t index = 0; index < indexBuffer.size(); index += TRIANGLE_STEPPING) {
		const int32_t vertexA = indexBuffer[index];
		const int32_t vertexB = indexBuffer[index + 1];
		const int32_t vertexC = indexBuffer[index + 2];

		Vector3 edgeAB = vertexBuffer[vertexB] - vertexBuffer[vertexA];
		Vector3 edgeAC = vertexBuffer[vertexC] - vertexBuffer[vertexA];

		// The cross product is perpendicular to both input vectors (normal to the plane).
		// Flip the argument order if you need the opposite winding.    
		Vector3 areaWeightedNormal = edgeAB.cross(edgeAC);

		// Don't normalize this vector just yet. Its magnitude is proportional to the
		// area of the triangle (times 2), so this helps ensure tiny/skinny triangles
		// don't have an outsized impact on the final normal per vertex.

		// Accumulate this cross product into each vertex normal slot.
		normalBuffer->insert(vertexA, normalBuffer->get(vertexA) += areaWeightedNormal);
		normalBuffer->insert(vertexB, normalBuffer->get(vertexB) += areaWeightedNormal);
		normalBuffer->insert(vertexC, normalBuffer->get(vertexC) += areaWeightedNormal);
	}

	// Finally, normalize all the sums to get a unit-length, area-weighted average.
	for (int32_t vertex = 0; vertex < vertexBuffer.size(); vertex++) {
		normalBuffer->insert(vertex, normalBuffer->get(vertex).normalized());
	}
	return Error::OK;
}

Error CesiumGDModelLoader::parse_gltf(const String& assetPath, CesiumGltfReader::GltfReaderResult* out)
{
	// Get the GLTF mesh
	Error err;
	Ref<FileAccess> assetRef = open_file_access_with_err(assetPath, FileAccess::READ, &err);

	if (err != Error::OK) {
		ERR_PRINT(String("Error loading gltf from disk: ") + REFLECT_ERR_NAME(err));
		return err;
	}

	// Get the raw data as a gsl span to pass it onto the GLTF reader for Cesium
	PackedByteArray rawData = assetRef->get_buffer(assetRef->get_length());
	std::byte* dataPtr = reinterpret_cast<std::byte*>(rawData.ptrw());
	std::span<const std::byte> dataSpan(dataPtr, rawData.size());

	CesiumGltfReader::GltfReader reader;
	CesiumGltfReader::GltfReaderOptions options = {};
	options.applyTextureTransform = true;
	options.decodeDraco = true;
	// options.decodeDraco = true;
	*out = reader.readGltf(dataSpan, options);
	ERR_FAIL_COND_V_MSG(!out->errors.empty(), Error::ERR_FILE_CORRUPT, "Cannot read 3D tile!");

	return OK;
}
