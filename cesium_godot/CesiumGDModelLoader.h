#ifndef CESIUM_GD_MODEL_LOADER_H
#define CESIUM_GD_MODEL_LOADER_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include "godot_cpp/classes/standard_material3d.hpp"
#include <godot_cpp/templates/vector.hpp>
using namespace godot;

namespace godot {
	class MeshInstance;
}
#elif defined(CESIUM_GD_MODULE)
#include <scene/resources/mesh.h>
class MeshInstance3D;
#endif

//Do not trust the compiler to delete this, we'll need it if REAL_T is double
#include "Utils/CesiumMathUtils.h"
#include "CesiumGltf/Model.h"
#include <CesiumGltfReader/GltfReader.h>


class CesiumGDModelLoader {
public:
	static Ref<ArrayMesh> generate_meshes_from_model(const CesiumGltf::Model& readerResult, Error* error);

	static Error parse_gltf(const String& assetPath, CesiumGltfReader::GltfReaderResult* out);

	template<class BT>
	static inline Vector<BT> get_attribute_from_primitive(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::Model& model, const std::string_view& attributeName, std::optional<std::function<void(BT&)>> callback = std::nullopt){

		Vector<BT> resultBuffer;
		const auto& attributeIterator = primitive.attributes.find(attributeName.data());
		if (attributeIterator == primitive.attributes.end()) {
			return Vector<BT>();
		}

		const CesiumGltf::Accessor& attrAccessor = model.accessors[attributeIterator->second];
		const CesiumGltf::BufferView& attrBufferView = model.bufferViews[attrAccessor.bufferView];
		const CesiumGltf::Buffer& attBuffer = model.buffers[attrBufferView.buffer];

		const std::byte* attrbuteData = &attBuffer.cesium.data[attrBufferView.byteOffset + attrAccessor.byteOffset];
		//If we have a double precision build, we need to account that in, as models will come with single precision attributes
#ifdef REAL_T_IS_DOUBLE
		//The destination here will be a vector of floats and then we cast that to the actual type
		constexpr size_t ATTR_SIZE = sizeof(BT) / 2;
#else

		constexpr size_t ATTR_SIZE = sizeof(BT);
#endif
		const int8_t componentSize = attrAccessor.computeByteSizeOfComponent();
		const int8_t componentCount = attrAccessor.computeNumberOfComponents();

		ERR_FAIL_COND_V_MSG((componentCount * componentSize) != ATTR_SIZE, resultBuffer, String("Attribute component length does not match template argument length"));

		int64_t stride = attrBufferView.byteStride.value_or(ATTR_SIZE);

		for (size_t i = 0; i < attrAccessor.count; ++i) {
			const std::byte* data = attrbuteData + i * stride;
			BT attribute = copy_to_buffer_type<BT>(data, ATTR_SIZE);
			if (callback) {
				(*callback)(attribute);
			}
			resultBuffer.push_back(attribute);
		}

		//Correct the attribute too
		while (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLES && resultBuffer.size() % 3 != 0) {
			resultBuffer.push_back(resultBuffer.get(resultBuffer.size() - 1)); // Duplicate the last normal
		}
		return resultBuffer;
	}

	static Error copy_material_properties(const CesiumGltf::Material& cesiumMaterial, Ref<StandardMaterial3D>& godotMaterial, const CesiumGltf::Model& modelReference);

	static Error apply_surface_to_mesh(const CesiumGltf::MeshPrimitive& meshPrimitive, Ref<ArrayMesh>& meshInstance, const Array& arrays);

private:

	static constexpr Mesh::PrimitiveType cesium_to_godot_primitive_mode(int32_t mode);

	static Vector<Vector3> generate_normals(const Vector<Vector3>& vertices, const Vector<int32_t>& indices);


	#if defined(CESIUM_GD_EXT)
	static Array generate_array_mesh_ext(const Vector<Vector3>& vertices, const Vector<int32_t>& indices, const Vector<Vector3>& normals, const Vector<Vector2>& textureCoords, const Vector<Vector2>& textureCoords2);
	#endif

	
	template<class T>
	static inline T copy_to_buffer_type(const std::byte* data, size_t attributeSize) {
#ifndef REAL_T_IS_DOUBLE
		//We can just copy as usual, real_t is float
		T attribute;
		//TODO: Check if we can just reinterpret cast here
		memcpy(&attribute, data, attributeSize);
		return attribute;
#else
		if constexpr (std::is_same<T, Vector2>::value) {
			const glm::vec2* tempAttribute = reinterpret_cast<const glm::vec2*>(data);
			Vector2 attribute = CesiumMathUtils::from_glm_vec2(*tempAttribute);
			return static_cast<T>(attribute);
		}
		else if constexpr (std::is_same<T, Vector3>::value) {
			//Vec3
			const glm::vec3* tempAttribute = reinterpret_cast<const glm::vec3*>(data);
			Vector3 attribute = CesiumMathUtils::from_glm_vec3(*tempAttribute);
			return static_cast<T>(attribute);
		}
#endif
	}

	static Vector<int32_t> get_index_buffer_from_primitive(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::Model& model, Error* error);

	static void set_colors_and_texture(const CesiumGltf::Material& cesiumMaterial, Ref<StandardMaterial3D>& godotMaterial, const CesiumGltf::Model& modelReference);

	static Error generate_normals(Vector<Vector3>* normalBuffer, const Vector<Vector3>& vertexBuffer, const Vector<int32_t>& indexBuffer, bool flip = false);
};

#endif // !CESIUM_GD_MODEL_LOADER_H
