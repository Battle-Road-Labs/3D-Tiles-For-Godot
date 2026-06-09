#include "GodotPrepareRenderResources.h"
#include "CesiumGltf/ImageAsset.h"
#include "CesiumGltfReader/ImageDecoder.h"
#include "Models/Cesium3DTile.h"
#include "Models/CesiumGlobe.h"
#include "Utils/CesiumVariantHash.h"
#include "error_names.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/fwd.hpp"
#include "godot_cpp/core/error_macros.hpp"

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shader.hpp>
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/shader.h"
#include "scene/resources/material.h"
using namespace godot;
#endif

#include "CesiumAsync/AsyncSystem.h"
#include "Cesium3DTilesSelection/Tile.h"
#include "../CesiumGDModelLoader.h"
#include "../Utils/CesiumGDTextureLoader.h"
#include "CesiumRasterOverlays/RasterOverlayTile.h"
#include "CesiumRasterOverlays/RasterOverlay.h"
#include "../Utils/CesiumMathUtils.h"
#include "CesiumGltf/Node.h"
#include "CesiumGltf/ExtensionModelExtStructuralMetadata.h"
#include <glm/gtc/quaternion.hpp>
#include "../Models/CesiumGDTileset.h"

using namespace CesiumAsync;
using namespace Cesium3DTilesSelection;

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources> GodotPrepareRenderResources::prepareInLoadThread(const CesiumAsync::AsyncSystem& asyncSystem, Cesium3DTilesSelection::TileLoadResult&& tileLoadResult, const glm::dmat4& transform, const std::any& rendererOptions)
{
	CesiumGltf::Model* model = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);

	if (model == nullptr) {
		return asyncSystem.createResolvedFuture(TileLoadResultAndRenderResources{ std::move(tileLoadResult), nullptr });
	}

	return asyncSystem.createFuture<TileLoadResultAndRenderResources>([=, this, tileLoadResult = std::move(tileLoadResult)](Promise<TileLoadResultAndRenderResources> p_promise) mutable {
		// Re-extract model pointer since tileLoadResult was moved
		CesiumGltf::Model* movedModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
		if (movedModel == nullptr) {
			ERR_PRINT("Model was invalidated after move!");
			TileLoadResultAndRenderResources errorResult{ std::move(tileLoadResult), nullptr };
			p_promise.resolve(errorResult);
			return;
		}

		Error err;
		Ref<ArrayMesh> meshData = CesiumGDModelLoader::generate_meshes_from_model(*movedModel, &err);

		if (err != Error::OK) {
			String errorMsg = String("Error generating meshes for tile ") + REFLECT_ERR_NAME(err);
			ERR_PRINT(errorMsg);
			// Always resolve with nullptr - reject() crashes with LIBASYNC_NO_EXCEPTIONS
			TileLoadResultAndRenderResources errorResult{ std::move(tileLoadResult), nullptr };
			p_promise.resolve(errorResult);
			return;
		}

		Cesium3DTile* instance = memnew(Cesium3DTile);
		instance->set_mesh(meshData);

		// Check if model has nodes before accessing
		if (movedModel->nodes.empty()) {
			ERR_PRINT("Tile model has no nodes - cannot render");
			TileLoadResultAndRenderResources errorResult{ std::move(tileLoadResult), nullptr };
			p_promise.resolve(errorResult);
			return;
		}

		const CesiumGltf::Node &rootNode = movedModel->nodes[0];

		const std::vector<double> &translationArray = rootNode.translation;
		const std::vector<double> &rotationArray = rootNode.rotation;
		const std::vector<double> &scaleArray = rootNode.scale;

		Transform3D gdTransform = Transform3D(Basis(), Vector3());
		gdTransform.scale(Vector3(1, 1, 1));
		instance->set_transform(gdTransform);

		// glTF scale is optional - default to 1,1,1 if not present
		Vector3 scale(1.0f, 1.0f, 1.0f);
		if (scaleArray.size() >= 3) {
			scale.x = scaleArray[0];
			scale.y = scaleArray[1];
			scale.z = scaleArray[2];
		}

		const glm::dmat4 transformationMat = CesiumMathUtils::array_to_dmat4(rootNode.matrix);

		glm::dvec3 glmPos;
		glm::dquat glmRot;
		// Applies for tilesets that 
		constexpr int32_t worldTerrainId = 1;
		constexpr int32_t osmBuildingsId = 96188;

		constexpr int32_t translationColumnIndex = 3;
		glmPos = transformationMat[translationColumnIndex];
		glmRot = glm::quat_cast(transformationMat);

		Vector3 translation;
		Quaternion rotation = CesiumMathUtils::from_glm_quat(glmRot);
		CesiumGeoreference* geoReferenceNode = nullptr;

		if (this->m_tileset->is_georeferenced(&geoReferenceNode)) {
			real_t scaleFactor = geoReferenceNode->get_scale_factor();
			instance->set_scale({ scaleFactor, scaleFactor, scaleFactor });
			translation *= scaleFactor;
		}

		// Avoid adding as child in the worker thread
		instance->set_tileset_no_reparent(this->m_tileset);

		// Save this for use later
		instance->set_original_position(glmPos);

		Vector3 eulerAngles = rotation.get_euler();

		translation = CesiumMathUtils::from_glm_vec3(glmPos);
		instance->set_position(translation);
		instance->set_rotation(eulerAngles);
		if (this->m_tileset->get_create_physics_meshes()) {
			instance->generate_tile_collision();
		}

		// Metadata extraction
		const CesiumGltf::ExtensionModelExtStructuralMetadata* modelMetadata = movedModel->getExtension<CesiumGltf::ExtensionModelExtStructuralMetadata>();
		instance->add_metadata(movedModel, modelMetadata);

		TileLoadResultAndRenderResources result{
			std::move(tileLoadResult),
			static_cast<void*>(instance)
		};

		p_promise.resolve(result);

	});
}

void* GodotPrepareRenderResources::prepareInMainThread(Tile& tile, void* pLoadThreadResult)
{
	// If load thread failed, pLoadThreadResult will be nullptr
	if (pLoadThreadResult == nullptr) {
		return nullptr;
	}

	const Cesium3DTilesSelection::TileContent& content = tile.getContent();
	const Cesium3DTilesSelection::TileRenderContent* pRenderContent = content.getRenderContent();

	if (pRenderContent == nullptr) {
		return pLoadThreadResult;
	}

	const CesiumGltf::Model& model = pRenderContent->getModel();
	// Apply the transform if any
	Cesium3DTile* instance = reinterpret_cast<Cesium3DTile*>(pLoadThreadResult);

	glm::dmat4 tileTransform = tile.getTransform();
	tileTransform = CesiumGDModelLoader::apply_rtc_center(model, tileTransform);
	tileTransform = CesiumGDModelLoader::apply_gltf_up_axis_transform(model, tileTransform);
	// Then apply the z up
	glm::dvec3 position = glm::dvec3(CesiumMathUtils::ecef_to_engine(tileTransform[3]));
	instance->set_original_position(instance->get_original_position() + position);
	return pLoadThreadResult;
}

void GodotPrepareRenderResources::free(Tile& tile, void* pLoadThreadResult, void* pMainThreadResult) noexcept
{
	const auto& tileId = tile.getTileID();
	uint64_t hash = static_cast<uint64_t>(std::visit(CesiumVariantHash{}, tileId));
	auto* instance = static_cast<Cesium3DTile*>(pMainThreadResult);
	this->m_tileset->call_deferred("free_tile", instance, hash);
}

void GodotPrepareRenderResources::attachRasterInMainThread(const Tile& tile, int32_t overlayTextureCoordinateID, const CesiumRasterOverlays::RasterOverlayTile& rasterTile, void* pMainThreadRendererResources, const glm::dvec2& translation, const glm::dvec2& scale)
{
	const Cesium3DTilesSelection::TileContent& content = tile.getContent();
	void* rawRenderResources = content.getRenderContent()->getRenderResources();
	auto* meshInstance = static_cast<Cesium3DTile*>(rawRenderResources);
	if (meshInstance == nullptr) return;

	Ref<ImageTexture> godotTexture = static_cast<ImageTexture*>(pMainThreadRendererResources);
	std::string key = rasterTile.getOverlay().getName();

	//Get all primitives (surfaces) in the mesh tile
	uint32_t primitiveIndex = 0;
	const CesiumGltf::Model& model = content.getRenderContent()->getModel();
	std::string overlayAttributeName = "_CESIUMOVERLAY_" + std::to_string(overlayTextureCoordinateID);

	Ref<ArrayMesh> arrayMesh = meshInstance->get_mesh();

	const Vector2 scaleFactors = CesiumMathUtils::from_glm_vec2(scale);
	const Vector2 translationOffsets = CesiumMathUtils::from_glm_vec2(translation);
	
	for (const CesiumGltf::Mesh& mesh : model.meshes) {
		for (const CesiumGltf::MeshPrimitive& primitive : mesh.primitives) {
			// We will allow this use of auto for now... for now
			auto attributeIt = primitive.attributes.find(overlayAttributeName);
			if (attributeIt == primitive.attributes.end()) {
				continue;
			}

			Array arrays = arrayMesh->surface_get_arrays(primitiveIndex);

			int32_t uvIndex = attributeIt->second;

			Ref<StandardMaterial3D> godotMaterial = memnew(StandardMaterial3D);
			const CesiumGltf::Material& mat = model.materials.at(primitive.material);
			Error err = CesiumGDModelLoader::copy_material_properties(mat, godotMaterial, model);
			godotMaterial->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, godotTexture);
			godotMaterial->set_uv1_scale(Vector3(scale.x, -scale.y, 1.0));
			godotMaterial->set_uv1_offset(Vector3(translation.x, 1 - translation.y, 1.0));
			if (err != Error::OK) {
				ERR_PRINT(String("Could not set texture for Raster Overlay, error: ") + REFLECT_ERR_NAME(err));
				continue;
			}

			//Apply the texture to the surface
			err = CesiumGDModelLoader::apply_surface_to_mesh(primitive, arrayMesh, arrays);
			if (err != Error::OK) {
				ERR_PRINT(String("Could not set texture for Raster Overlay, error: ") + REFLECT_ERR_NAME(err));
				continue;
			}

			Ref<Material> finalMaterial = godotMaterial;
#ifdef __EMSCRIPTEN__
			// Raster-overlay path replaces the ShaderMaterial from the initial mesh load,
			// so we have to re-apply the web amplification here. Mirror the uv1 offset/scale
			// the StandardMaterial3D uses (see above) as shader parameters. Pick the cull
			// variant matching the source glTF material so photogrammetry buildings
			// (doubleSided=true) don't get wrong-side-culled by cull_front.
			Ref<Shader> tile_shader = CesiumGDModelLoader::get_tile_shader(mat.doubleSided);
			if (tile_shader.is_valid()) {
				Ref<ShaderMaterial> shaderMat;
				shaderMat.instantiate();
				shaderMat->set_shader(tile_shader);
				shaderMat->set_shader_parameter("albedo_texture", godotTexture);
				shaderMat->set_shader_parameter("uv_scale", Vector3(scale.x, -scale.y, 1.0));
				shaderMat->set_shader_parameter("uv_offset", Vector3(translation.x, 1.0 - translation.y, 0.0));
				shaderMat->set_shader_parameter("uv_rotation", 0.0f);
				shaderMat->set_shader_parameter("albedo_amplification", 3.0f);
				// Web-only texture-modulated ambient emission so dark overlays
				// (ocean imagery, shadow-side terrain) don't get crushed by the
				// scene's aggressive tonemap. Kept at 1.0 to match the initial
				// mesh-load path in CesiumGDModelLoader; tune both together.
				shaderMat->set_shader_parameter("ambient_level", 1.0f);
				finalMaterial = shaderMat;
			}
#endif
			arrayMesh->surface_set_material(primitiveIndex, finalMaterial);

			primitiveIndex++;
		}
	}
	meshInstance->set_mesh(arrayMesh);
}

void GodotPrepareRenderResources::detachRasterInMainThread(const Tile& tile, int32_t overlayTextureCoordinateID, const CesiumRasterOverlays::RasterOverlayTile& rasterTile, void* pMainThreadRendererResources) noexcept
{

}

void* GodotPrepareRenderResources::prepareRasterInLoadThread(CesiumGltf::ImageAsset& image, const std::any& rendererOptions)
{
	//I guess we just generate mip maps here (?
	CesiumGltfReader::ImageDecoder::generateMipMaps(image);
	return nullptr;
}

void* GodotPrepareRenderResources::prepareRasterInMainThread(CesiumRasterOverlays::RasterOverlayTile& rasterTile, void* pLoadThreadResult)
{
	const CesiumGltf::ImageAsset& imageCesium = *rasterTile.getImage().get();
	Ref<ImageTexture> godotTexture = CesiumGDTextureLoader::load_image_texture(imageCesium, false, true);
	godotTexture->reference();
	return static_cast<void*>(godotTexture.ptr());
}

void GodotPrepareRenderResources::freeRaster(const CesiumRasterOverlays::RasterOverlayTile& rasterTile, void* pLoadThreadResult, void* pMainThreadResult) noexcept
{
	auto* rasterMainThreadTexture = static_cast<ImageTexture*>(pMainThreadResult);
	if (rasterMainThreadTexture == nullptr) return;
	rasterMainThreadTexture->unreference();
}
