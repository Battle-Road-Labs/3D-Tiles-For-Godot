#include "Models/CesiumGDCreditSystem.h"
#include "godot_cpp/classes/resource_loader.hpp"
#define SPDLOG_COMPILED_LIB
#include "Models/CesiumGlobe.h"
#define SPDLOG_FMT_EXTERNAL

#include "CesiumGDTileset.h"

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/core/property_info.hpp"
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics/collision_object_3d.h"
#include "scene/3d/camera_3d.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "scene/3d/physics/collision_shape_3d.h"
#include "core/error/error_macros.h"
#endif


#include "Utils/AssetManipulation.h"
#include "Cesium3DTilesSelection/Tileset.h"
#include "../CesiumGDModelLoader.h"
#include "Cesium3DTilesSelection/TilesetExternals.h"
#include "../Implementations/LocalAssetAccesor.h"
#include "SimpleTaskProcessor.h"
#include "../Utils/CesiumMathUtils.h"
#include "../Implementations/NetworkAssetAccessor.h"
#include "../Implementations/GodotPrepareRenderResources.h"
#include "CesiumHTTPRequestNode.h"
#include "Cesium3DTilesContent/registerAllTileContentTypes.h"
#include "../../CesiumGeometry/include/CesiumGeometry/QuadtreeTileID.h"
#include "../Utils/CesiumVariantHash.h"
#include <glm/gtc/quaternion.hpp>
#include "CesiumGDRasterOverlay.h"
#include "../Utils/NetworkUtils.h"
#include <algorithm>
#include <execution>
#include "CesiumGDGeoreference.h"
#include <CesiumAsync/CachingAssetAccessor.h>
#include "CesiumAsync/SqliteCache.h"


static int32_t tileCount = 0;

constexpr real_t LOADING_LIMIT_SECONDS = 5.0;
constexpr const char* ION_ACCESS_TOKEN_P_NAME = "ion_access_token";
constexpr const char* ION_ASSET_ID_P_NAME = "ion_asset_id";
constexpr const char* URL_P_NAME = "url";

/**
* @brief This will be the underlying config for the tileset
* the class basically acts as a builder wrapper to provide
* UI serialization in-engine
*/
class OpaqueTilesetOptions {
public:
	Cesium3DTilesSelection::TilesetOptions options{};

	Cesium3DTilesSelection::TilesetContentOptions contentOptions{};
};

CesiumGDTileset::CesiumGDTileset()
{
	this->m_initialLoadingFinished = false;
	this->m_tilesetConfig = new OpaqueTilesetOptions();
	//Set all the default values for the tileset options that are not exposed to the editor
	this->m_tilesetConfig->options.mainThreadLoadingTimeLimit = LOADING_LIMIT_SECONDS;
	this->m_tilesetConfig->options.tileCacheUnloadTimeLimit = LOADING_LIMIT_SECONDS;
	this->m_tilesetConfig->contentOptions.applyTextureTransform = false;
	CesiumGltf::SupportedGpuCompressedPixelFormats supportedFormats;

	supportedFormats.ETC1_RGB = true;
	supportedFormats.BC1_RGB = true;
	supportedFormats.BC3_RGBA = true;
	supportedFormats.ASTC_4x4_RGBA = true;
	supportedFormats.ETC2_EAC_R11 = true;
	supportedFormats.ETC2_EAC_RG11 = true;
	supportedFormats.BC7_RGBA = true;

	this->m_tilesetConfig->contentOptions.ktx2TranscodeTargets = { supportedFormats, true };

	this->m_tilesetConfig->options.loadErrorCallback = [=](const Cesium3DTilesSelection::TilesetLoadFailureDetails& failData) {
		ERR_PRINT(String("Failed to load a given tileset, error: ") + failData.message.c_str());
	};

	this->m_signalingThreadPool.init(5);

	Cesium3DTilesContent::registerAllTileContentTypes();
}

void CesiumGDTileset::set_maximum_screen_space_error(real_t error)
{
	this->m_tilesetConfig->options.maximumScreenSpaceError = error;
}

real_t CesiumGDTileset::get_maximum_screen_space_error() const
{
	return this->m_tilesetConfig->options.maximumScreenSpaceError;
}

void CesiumGDTileset::set_maximum_simultaneous_tile_loads(uint32_t count)
{
	this->m_tilesetConfig->options.maximumSimultaneousTileLoads = count;
}

uint32_t CesiumGDTileset::get_maximum_simultaneous_tile_loads() const
{
	return this->m_tilesetConfig->options.maximumSimultaneousTileLoads;
}

void CesiumGDTileset::set_preload_ancestors(bool preload)
{
	this->m_tilesetConfig->options.preloadAncestors = preload;
}

bool CesiumGDTileset::get_preload_ancestors() const
{
	return this->m_tilesetConfig->options.preloadAncestors;
}

void CesiumGDTileset::set_preload_siblings(bool preload)
{
	this->m_tilesetConfig->options.preloadSiblings = preload;
}

bool CesiumGDTileset::get_preload_siblings() const
{
	return this->m_tilesetConfig->options.preloadSiblings;
}

void CesiumGDTileset::set_loading_descendant_limit(uint32_t limit)
{
	this->m_tilesetConfig->options.loadingDescendantLimit = limit;
}

uint32_t CesiumGDTileset::get_loading_descendant_limit() const
{
	return this->m_tilesetConfig->options.loadingDescendantLimit;
}

void CesiumGDTileset::set_forbid_holes(bool forbidHoles)
{
	this->m_tilesetConfig->options.forbidHoles = forbidHoles;
}

bool CesiumGDTileset::get_forbid_holes() const
{
	return this->m_tilesetConfig->options.forbidHoles;
}

void CesiumGDTileset::set_url(const String& url)
{
	//Assert the source
	this->m_url = url;
	this->recreate_tileset();
}

const String& CesiumGDTileset::get_url() const
{
	return this->m_url;
}

void CesiumGDTileset::set_generate_missing_normals_smooth(bool shouldGenerate)
{
	this->m_tilesetConfig->contentOptions.generateMissingNormalsSmooth = shouldGenerate;
}


bool CesiumGDTileset::get_generate_missing_normals_smooth() const
{
	return this->m_tilesetConfig->contentOptions.generateMissingNormalsSmooth;
}

int CesiumGDTileset::get_data_source() const
{
	return static_cast<int>(this->m_selectedDataSource);
}

void CesiumGDTileset::set_data_source(int data_source)
{
	this->m_selectedDataSource = static_cast<CesiumDataSource>(data_source);
	this->notify_property_list_changed();
}

void CesiumGDTileset::set_ion_asset_id(int64_t id)
{
	this->m_cesiumIonAssetId = id;
}

int64_t CesiumGDTileset::get_ion_asset_id() const
{
	return this->m_cesiumIonAssetId;
}

Ref<CesiumGDConfig> CesiumGDTileset::get_cesium_config() const
{
	return this->m_configInstance;
}

void CesiumGDTileset::set_cesium_config(const Ref<CesiumGDConfig>& config)
{
	this->m_configInstance = config;
}


void CesiumGDTileset::set_create_physics_meshes(bool shouldCreate)
{
	this->m_createPhysicsMeshes = shouldCreate;
}

bool CesiumGDTileset::get_create_physics_meshes() const
{
	return this->m_createPhysicsMeshes;
}

void CesiumGDTileset::update_tileset(const Transform3D& cameraTransform)
{
	
	bool isGeoreferenced = this->is_georeferenced(&this->m_georeference);
	if (this->m_activeTileset == nullptr) {
		if (isGeoreferenced) {
			this->m_georeference->set_should_update_origin(true);
			this->m_georeference->move_origin();
		}
		
		this->load_tileset();
	}

	//Get the camera view state
	Viewport* currentViewport = get_viewport();
	Camera3D* cam = currentViewport->get_camera_3d();

	glm::dvec3 camPos;
	if (isGeoreferenced) {
		// Possible optimization opportunity by calculating the transformation using strictly GLM
		// OR even skipping all transformations, and instead just get the "equivalent" up and direction vectors for the cam
		// I like the second idea more 
		camPos = this->m_georeference->get_ecef_position();
	}
	else {
		camPos = CesiumMathUtils::to_glm_dvec3(cameraTransform.origin);
	}

	Vector3 cameraDirectionGodot = cameraTransform.basis.get_column(Vector3::AXIS_Z);
	Vector3 cameraUp = cameraTransform.basis.get_column(Vector3::AXIS_Y);
	glm::dvec3 cameraDirection = glm::normalize(CesiumMathUtils::to_glm_dvec3(-cameraDirectionGodot));


	Vector2 viewportSize;
	#if defined(CESIUM_GD_EXT)
	viewportSize = currentViewport->get_visible_rect().get_size();
	#elif defined(CESIUM_GD_MODULE)
	viewportSize = currentViewport->get_camera_rect_size();
	#endif
	
	Vector2 aspectRatioV = viewportSize;
	real_t aspectRatioF = aspectRatioV.x / aspectRatioV.y;

	double verticalFOV = Math::deg_to_rad(cam->get_fov()); //TODO: Probably load 1.2x of the FOV for other tiles?
	double horizontalFOV = 2 * Math::atan(aspectRatioF * Math::tan(verticalFOV * 0.5));

	Cesium3DTilesSelection::ViewState currentViewState = Cesium3DTilesSelection::ViewState::create(
		camPos,
		cameraDirection,
		CesiumMathUtils::to_glm_dvec3(cameraUp),
		CesiumMathUtils::to_glm_vec2(viewportSize),
		horizontalFOV * 1.2f,
		verticalFOV * 1.2f
	);

	const Cesium3DTilesSelection::ViewUpdateResult& updateResult = this->m_activeTileset->updateView({ currentViewState });

	for (Cesium3DTilesSelection::Tile *tile : updateResult.tilesToRenderThisFrame) {
		this->render_tile_as_node(*tile);
	}

	for (Cesium3DTilesSelection::Tile *tile : updateResult.tilesFadingOut) {
		despawn_tile(*tile);
	}
}

CesiumHTTPRequestNode* CesiumGDTileset::get_available_request_node() noexcept
{
	//TODO: Remove this func
	return nullptr;
}

bool CesiumGDTileset::is_initial_loading_finished() const
{
	return this->m_initialLoadingFinished;
}

Vector3 CesiumGDTileset::get_earth_origin() const
{
	//The origin (for now at least) will be definined by
	//the zero vector - the Z component (in Cesium Y is depth, this is equivalent to Godot's Z) of the radius, scaled down
	return Vector3(0.0, 0.0, -CesiumGeospatial::Ellipsoid::WGS84.getRadii().y);
}

void CesiumGDTileset::add_overlay(CesiumGDRasterOverlay* overlay)
{
	if (overlay == nullptr) return;
	this->m_activeTileset->getOverlays().add(overlay->get_overlay_instance());
}


bool CesiumGDTileset::is_georeferenced(CesiumGlobe** outRef) const
{
	if (this->m_georeference != nullptr) {
		*outRef = this->m_georeference;
		return this->m_georeference->get_origin_type() == static_cast<int32_t>(CesiumGlobe::OriginType::CartographicOrigin);
	}
	//Check if the parent is of type CesiumGDGeoreference
	Node3D* parent = this->get_parent_node_3d();
	*outRef = Object::cast_to<CesiumGlobe>(parent);
	return (*outRef)->get_origin_type() == static_cast<int32_t>(CesiumGlobe::OriginType::CartographicOrigin);
}

void CesiumGDTileset::recreate_tileset()
{
}

void CesiumGDTileset::load_tileset()
{
	//Get the options to read the tileset and then load it into memory
	const Cesium3DTilesSelection::TilesetOptions& options = this->m_tilesetConfig->options;
	const Cesium3DTilesSelection::TilesetContentOptions& contentOptions = this->m_tilesetConfig->contentOptions;

	if (this->m_selectedDataSource == CesiumDataSource::FromCesiumIon) {
		this->m_activeTileset = std::make_unique<Cesium3DTilesSelection::Tileset>(
			this->create_tileset_externals(),
			this->m_cesiumIonAssetId,
			this->m_configInstance->get_access_token().utf8().get_data(),
			options,
			this->m_configInstance->get_api_url().utf8().get_data()
		);
	}
	//Else this is coming from a URL
	else {
		this->m_activeTileset = std::make_unique<Cesium3DTilesSelection::Tileset>(
			this->create_tileset_externals(),
			this->m_url.utf8().get_data(),
			options
		);
	}

	int32_t childCount = this->get_child_count();
	for (int32_t i = 0; i < childCount; i++)
	{
		Node* currChild = this->get_child(i);
		CesiumGDRasterOverlay* overlay = Object::cast_to<CesiumGDRasterOverlay>(currChild);
		if (overlay == nullptr) continue;
		overlay->add_to_tileset(this);
	}

}

Cesium3DTilesSelection::TilesetExternals CesiumGDTileset::create_tileset_externals()
{
	const String cachePath = "user://cache";
	Ref<DirAccess> userAccess = DirAccess::open("user://");
	Error err = userAccess->make_dir_recursive(cachePath);
	if (err != Error::OK) {
		ERR_PRINT("Could not create / use temporary cache path!");
	}
	String globalCachePath = ProjectSettings::get_singleton()->globalize_path(cachePath) + "/cesium-request-cache.sqlite";
	constexpr int32_t requestsPerCachePrune = 10000;
	constexpr uint64_t maxItems = 4096;

	auto cache = std::make_shared<CesiumAsync::SqliteCache>(spdlog::default_logger(), globalCachePath.utf8().get_data(), maxItems);
	auto simpleAccessor = std::make_shared<NetworkAssetAccessor>();
	auto cachedAccessor = std::make_shared<CesiumAsync::CachingAssetAccessor>(spdlog::default_logger(), simpleAccessor, cache, requestsPerCachePrune);
	
	auto taskProcessor = std::make_shared<SimpleTaskProcessor>();
	CesiumAsync::AsyncSystem asyncSystem(taskProcessor);
	auto renderResourcesProvider = std::make_shared<GodotPrepareRenderResources>(this);
	auto creditSystem = std::make_shared<CesiumUtility::CreditSystem>();
	CesiumGDCreditSystem::get_singleton(this)->add_credit_system(creditSystem);
	
	Cesium3DTilesSelection::TilesetExternals result {
		cachedAccessor,
		renderResourcesProvider,
		asyncSystem,
		creditSystem
	};
	return result;
}

void CesiumGDTileset::render_tile_as_node(const Cesium3DTilesSelection::Tile& tile)
{
	//Check if it's already in the scene tree first
	const Cesium3DTilesSelection::TileID& tileId = tile.getTileID();
	size_t hash = std::visit(CesiumVariantHash{}, tileId);

	if (this->m_instancedTilesByHash.find(hash) != this->m_instancedTilesByHash.end()) {
		this->m_initialLoadingFinished = true;
		MeshInstance3D* foundNode = m_instancedTilesByHash[hash];
		if (foundNode == nullptr) {
			WARN_PRINT("Failed to get a tile from the instance id");
			return;
		}
		foundNode->show();

		if (this->m_createPhysicsMeshes) {
			Node* collisionNode = foundNode->get_child_count() < 1 ? nullptr : foundNode->get_child(0);
			if (collisionNode == nullptr) return;
			CollisionShape3D* shape = collisionNode->get_child_count() < 1 ? nullptr : Object::cast_to<CollisionShape3D>(collisionNode->get_child(0));
			if (shape == nullptr) return;
			shape->set_disabled(false);
		}

		return;
	}

	bool renderable = tile.isRenderable();
	bool empty = tile.isEmptyContent();
	if (!tile.isRenderable() || tile.getContent().isEmptyContent()) return;

	if (tile.getState() == Cesium3DTilesSelection::TileLoadState::Failed) {
		std::string tileIdStr = Cesium3DTilesSelection::TileIdUtilities::createTileIdString(tile.getTileID());
		ERR_PRINT(String("Failed to load tile ") + tileIdStr.c_str());
		return;
	}

	Error err;
	const CesiumGltf::Model& model = tile.getContent().getRenderContent()->getModel();

	MeshInstance3D* instance = static_cast<MeshInstance3D*>(tile.getContent().getRenderContent()->getRenderResources());
	if (instance == nullptr) {
		//FIXME: This might actually have a bug
		//If we didn't load a render resource, create the new mesh (we'll register this to cache later)
		Ref<ArrayMesh> meshData = CesiumGDModelLoader::generate_meshes_from_model(model, &err);
		if (err != Error::OK) {
			ERR_PRINT(String("Could not generate render resources for tile: ") + itos(hash));
			return;
		}
		instance = memnew(MeshInstance3D);
		instance->set_mesh(meshData);
	}

	this->register_tile(instance, hash);
	instance->set_name(itos(hash));
}

void CesiumGDTileset::despawn_tile(const Cesium3DTilesSelection::Tile& tile)
{
	size_t hash = std::visit(CesiumVariantHash{}, tile.getTileID());

	if (this->m_instancedTilesByHash.find(hash) == this->m_instancedTilesByHash.end()) {
		return;
	}

	MeshInstance3D* foundNode = this->m_instancedTilesByHash[hash];
	if (foundNode == nullptr) {
		WARN_PRINT("Failed to despawn tile, address is nullptr");
		return;
	}
	
	const Cesium3DTilesSelection::TileRenderContent* renderContent = tile.getContent().getRenderContent();
	
	foundNode->hide();
	// Deactivate the collisions
	if (this->m_createPhysicsMeshes) {
		Node* collisionNode = foundNode->get_child_count() < 1 ? nullptr : foundNode->get_child(0);
		if (collisionNode == nullptr) return;
			CollisionShape3D* shape = collisionNode->get_child_count() < 1 ? nullptr : Object::cast_to<CollisionShape3D>(collisionNode->get_child(0));
			if (shape == nullptr) return;
			shape->set_disabled(true);
	}
}

void CesiumGDTileset::despawn_tile_deferred(const Cesium3DTilesSelection::Tile& tile)
{
	size_t hash = std::visit(CesiumVariantHash{}, tile.getTileID());

	if (this->m_instancedTilesByHash.find(hash) == this->m_instancedTilesByHash.end()) {
		std::string tileIdStr = Cesium3DTilesSelection::TileIdUtilities::createTileIdString(tile.getTileID());
		ERR_PRINT(String("Could not find node with name ") + itos(hash));
		return;
	}

	
	MeshInstance3D* foundNode = this->m_instancedTilesByHash[hash];
	if (foundNode != nullptr && foundNode->is_inside_tree()) {
		return;
	}
	foundNode->call_deferred("hide");
}

bool CesiumGDTileset::try_get_tile_from_instance_id(const ObjectID& objectId, MeshInstance3D** outNode)
{
	*outNode = Object::cast_to<MeshInstance3D>(ObjectDB::get_instance(objectId));
	return *outNode != nullptr;
}


void CesiumGDTileset::process_tile_chunk(const std::vector<Cesium3DTilesSelection::Tile*> &tilesView, int32_t offset, int32_t size) {
	// NOTE: This function is meant to run on a worker thread
	// Given a certain buffer window for our chunk, we can access the memory and render those tiles individually
	for (int32_t i = offset; i < size; i++)
	{
		this->render_tile_as_node(*tilesView.at(i));
	}
}


void CesiumGDTileset::register_tile(MeshInstance3D *instance, size_t hash) {
	auto internalMode = this->m_showHierarchy ? Node::InternalMode::INTERNAL_MODE_DISABLED : Node::InternalMode::INTERNAL_MODE_FRONT; 
	this->add_child(instance, false, internalMode);
	this->m_instancedTilesByHash.insert({ hash, instance });
	tileCount++;
}


bool CesiumGDTileset::get_show_hierarchy() const {
	return this->m_showHierarchy;
}

void CesiumGDTileset::set_show_hierarchy(bool show) {
	this->m_showHierarchy = show;
}

void CesiumGDTileset::_bind_methods()
{
#pragma region Inspector properties
	//TODO: Maybe make some abstraction for this lol
	ClassDB::bind_method(D_METHOD("set_maximum_screen_space_error", "error"), &CesiumGDTileset::set_maximum_screen_space_error);
	ClassDB::bind_method(D_METHOD("get_maximum_screen_space_error"), &CesiumGDTileset::get_maximum_screen_space_error);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "maximum_screen_space_error"), "set_maximum_screen_space_error", "get_maximum_screen_space_error");

	ClassDB::bind_method(D_METHOD("set_maximum_simultaneous_tile_loads", "count"), &CesiumGDTileset::set_maximum_simultaneous_tile_loads);
	ClassDB::bind_method(D_METHOD("get_maximum_simultaneous_tile_loads"), &CesiumGDTileset::get_maximum_simultaneous_tile_loads);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "maximum_simultaneous_tile_loads"), "set_maximum_simultaneous_tile_loads", "get_maximum_simultaneous_tile_loads");

	ClassDB::bind_method(D_METHOD("set_preload_ancestors", "preload"), &CesiumGDTileset::set_preload_ancestors);
	ClassDB::bind_method(D_METHOD("get_preload_ancestors"), &CesiumGDTileset::get_preload_ancestors);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preload_ancestors"), "set_preload_ancestors", "get_preload_ancestors");

	ClassDB::bind_method(D_METHOD("set_preload_siblings", "preload"), &CesiumGDTileset::set_preload_siblings);
	ClassDB::bind_method(D_METHOD("get_preload_siblings"), &CesiumGDTileset::get_preload_siblings);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preload_siblings"), "set_preload_siblings", "get_preload_siblings");

	ClassDB::bind_method(D_METHOD("set_loading_descendant_limit", "limit"), &CesiumGDTileset::set_loading_descendant_limit);
	ClassDB::bind_method(D_METHOD("get_loading_descendant_limit"), &CesiumGDTileset::get_loading_descendant_limit);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "loading_descendant_limit"), "set_loading_descendant_limit", "get_loading_descendant_limit");

	ClassDB::bind_method(D_METHOD("set_forbid_holes", "forbidHoles"), &CesiumGDTileset::set_forbid_holes);
	ClassDB::bind_method(D_METHOD("get_forbid_holes"), &CesiumGDTileset::get_forbid_holes);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "forbid_holes"), "set_forbid_holes", "get_forbid_holes");

	ClassDB::bind_method(D_METHOD("set_generate_missing_normals_smooth", "shouldGenerate"), &CesiumGDTileset::set_generate_missing_normals_smooth);
	ClassDB::bind_method(D_METHOD("get_generate_missing_normals_smooth"), &CesiumGDTileset::get_generate_missing_normals_smooth);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "generate_missing_normals_smooth"), "set_generate_missing_normals_smooth", "get_generate_missing_normals_smooth");

	ClassDB::bind_method(D_METHOD("set_create_physics_meshes", "shouldGenerate"), &CesiumGDTileset::set_create_physics_meshes);
	ClassDB::bind_method(D_METHOD("get_create_physics_meshes"), &CesiumGDTileset::get_create_physics_meshes);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "create_physics_meshes"), "set_create_physics_meshes", "get_create_physics_meshes");

	ClassDB::bind_method(D_METHOD("get_cesium_config"), &CesiumGDTileset::get_cesium_config);
	ClassDB::bind_method(D_METHOD("set_cesium_config", "cesiumConfig"), &CesiumGDTileset::set_cesium_config);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cesiumConfig", PROPERTY_HINT_RESOURCE_TYPE, "CesiumGDConfig"), "set_cesium_config", "get_cesium_config");


	ClassDB::bind_method(D_METHOD("get_data_source"), &CesiumGDTileset::get_data_source);
	ClassDB::bind_method(D_METHOD("set_data_source", "data_source"), &CesiumGDTileset::set_data_source);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "data_source", PROPERTY_HINT_ENUM, "From Cesium Ion,From Url"), "set_data_source", "get_data_source");
	BIND_ENUM_CONSTANT(static_cast<int64_t>(CesiumDataSource::FromCesiumIon));
	BIND_ENUM_CONSTANT(static_cast<int64_t>(CesiumDataSource::FromUrl));

	ClassDB::bind_method(D_METHOD("set_url", URL_P_NAME), &CesiumGDTileset::set_url);
	ClassDB::bind_method(D_METHOD("get_url"), &CesiumGDTileset::get_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "url"), "set_url", "get_url");

	ClassDB::bind_method(D_METHOD("set_ion_asset_id", ION_ASSET_ID_P_NAME), &CesiumGDTileset::set_ion_asset_id);
	ClassDB::bind_method(D_METHOD("get_ion_asset_id"), &CesiumGDTileset::get_ion_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ion_asset_id"), "set_ion_asset_id", "get_ion_asset_id");


	ClassDB::bind_method(D_METHOD("set_show_hierarchy", "showHierarchy"), &CesiumGDTileset::set_show_hierarchy);
	ClassDB::bind_method(D_METHOD("get_show_hierarchy"), &CesiumGDTileset::get_show_hierarchy);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_hierarchy"), "set_show_hierarchy", "get_show_hierarchy");
	
#pragma endregion

#pragma region Public methods
	ClassDB::bind_method(D_METHOD("is_initial_loading_finished"), &CesiumGDTileset::is_initial_loading_finished);
	ClassDB::bind_method(D_METHOD("update_tileset", "camera_transform"), &CesiumGDTileset::update_tileset);
	ClassDB::bind_method(D_METHOD("get_earth_origin"), &CesiumGDTileset::get_earth_origin);
#pragma endregion
}

void CesiumGDTileset::_get_property_list(List<PropertyInfo>* properties) const
{
	#if defined(CESIUM_GD_MODULE)
	for (int32_t i = 0; i < properties->size(); i++) {
		PropertyInfo& propertyRef = properties->get(i);
	#elif defined(CESIUM_GD_EXT)
	for (auto it = properties->begin(); it != properties->end(); ++it) {
		PropertyInfo& propertyRef = *it;
	#endif
		propertyRef.usage = this->update_property_usage_flags(propertyRef);
	}
}


uint32_t CesiumGDTileset::update_property_usage_flags(const PropertyInfo& propertyRef) const
{	
	const String urlNameProp = "url";
	const String assetIdNameProp = "ion_asset_id";
	
	if (propertyRef.name == urlNameProp) {
		return this->m_selectedDataSource == CesiumDataSource::FromCesiumIon ? PROPERTY_USAGE_READ_ONLY : PROPERTY_USAGE_DEFAULT;
	}
	if (propertyRef.name == assetIdNameProp) {
		return this->m_selectedDataSource == CesiumDataSource::FromCesiumIon ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_READ_ONLY;
	}
	return propertyRef.usage;
}

bool CesiumGDTileset::_set(const StringName& p_name, const Variant& p_property)
{
	if (p_name == StringName(URL_P_NAME)) {
		this->set_url(p_property);
		return true;
	}
	if (p_name == StringName(ION_ASSET_ID_P_NAME)) {
		this->set_ion_asset_id(p_property);
		return true;
	}
	return false;
}

bool CesiumGDTileset::_get(const StringName& p_name, Variant& r_property) const
{
	if (p_name == StringName(URL_P_NAME)) {
		r_property = this->get_url();
		return true;
	}
	if (p_name == StringName(ION_ASSET_ID_P_NAME)) {
		r_property = this->get_ion_asset_id();
		return true;
	}

	return false;
}

void CesiumGDTileset::_enter_tree() {
	if (!is_editor_mode()) return;
	CesiumGlobe* globe = Godot3DTiles::AssetManipulation::find_or_create_globe(this);
	//Parent to the globe
	this->reparent(globe, true);
	this->set_rotation_degrees(Vector3(90.0, 0.0, 0.0));
	this->set_owner(globe->get_parent_node_3d());
	if (this->m_configInstance == nullptr) {
		this->m_configInstance = ResourceLoader::get_singleton()->load("res://addons/cesium_godot/cesium_gd_config.tres");
	}
}
	

