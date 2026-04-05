#include "Cesium3DTilesSelection/BoundingVolume.h"
#include "Cesium3DTilesSelection/ViewState.h"
#include "CesiumGeometry/BoundingCylinderRegion.h"
#include "CesiumGeometry/BoundingSphere.h"
#include "CesiumGeometry/OrientedBoundingBox.h"
#include "CesiumGeospatial/BoundingRegion.h"
#include "CesiumGeospatial/BoundingRegionWithLooseFittingHeights.h"
#include "CesiumGeospatial/S2CellBoundingVolume.h"
#include "CesiumUtility/IntrusivePointer.h"
#include "Models/CesiumDataSource.h"
#include "Models/GeoreferencedNode.h"
#include "glm/ext/matrix_double4x4.hpp"
#include "glm/ext/vector_double3.hpp"
#include "godot_cpp/classes/geometry_instance3d.hpp"
#include "godot_cpp/variant/basis.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/quaternion.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <cstdint>
#include <type_traits>
#define SPDLOG_COMPILED_LIB
#include "Models/CesiumGlobe.h"
#define SPDLOG_FMT_EXTERNAL

#include "Models/CesiumGDCreditSystem.h"
#include "missing_functions.hpp"
#include "Cesium3DTilesSelection/Tile.h"
#include "Cesium3DTilesSelection/TileContent.h"
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


#include "Models/Cesium3DTile.h"
#include "Utils/AssetManipulation.h"
#include "Cesium3DTilesSelection/Tileset.h"
#include "Cesium3DTilesSelection/TilesetExternals.h"
#include "SimpleTaskProcessor.h"
#include "../Utils/CesiumMathUtils.h"
#include "../Implementations/NetworkAssetAccessor.h"
#include "../Implementations/GodotPrepareRenderResources.h"
#include "Cesium3DTilesContent/registerAllTileContentTypes.h"
#include "../Utils/CesiumVariantHash.h"
#include <glm/gtc/quaternion.hpp>
#include "CesiumGDRasterOverlay.h"
#include "CesiumAsync/GunzipAssetAccessor.h"
#include <CesiumAsync/CachingAssetAccessor.h>
#include "CesiumAsync/SqliteCache.h"


static int32_t tileCount = 0;

constexpr real_t LOADING_LIMIT_SECONDS = 5.0;
constexpr const char* ION_ACCESS_TOKEN_P_NAME = "ion_access_token";
constexpr const char* ION_ASSET_ID_P_NAME = "ion_asset_id";
constexpr const char* URL_P_NAME = "url";

constexpr const char* MAXIMUM_SCREEN_SPACE_DESC = "The maximum number of pixels of error when rendering this tileset.\nThis is used to select an appropriate level-of-detail.\n\nWhen a tileset uses the older layer.json / quantized-mesh format rather than 3D Tiles, this value is effectively divided by 8.0.\nSo the default value of 16.0 corresponds to the standard value for quantized-mesh terrain of 2.0";
constexpr const char* MAXIMUM_SIMULTANEOUS_TILE_LOADS_DESC = "The maximum number of tiles that may simultaneously be in the process of loading.";
constexpr const char* PRELOAD_ANCESTORS_DESC = "Indicates whether the ancestors of rendered tiles should be preloaded.\nSetting this to true optimizes the zoom-out experience and provides more detail in newly-exposed areas when panning.\nThe down side is that it requires loading more tiles";
constexpr const char* PRELOAD_SIBLINGS_DESC = "Indicates whether the siblings of rendered tiles should bepreloaded.\nSetting this to true causes tiles with the same parent as arendered tile to be loaded, even if they are culled.\nSetting this to truemay provide a better panning experience at the cost of loading more tiles.";
constexpr const char* LOADING_DESCENDANT_LIMIT_DESC = "The number of loading descendant tiles that is considered \"too many\".\nIf a tile has too many loading descendants, that tile will be loaded and rendered before any of its descendants are loaded and rendered. \nThis means more feedback for the user that something is happening at the cost of a longer overall load time.\nSetting this to 0 will cause each tile level to be loaded successively, significantly increasing load time.\nSetting it to a large number (e.g. 1000) will minimize the number of tiles that are loaded but tend to make detail appear all at once after a long wait.";
constexpr const char* FORBID_HOLES_DESC = "Never render a tileset with missing tiles.\n\nWhen true, the tileset will guarantee that the tileset will never be rendered with holes in place of tiles that are not yet loaded.\nIt does this by refusing to refine a parent tile until all of its child tiles are ready to render.\nThus, when the camera moves, we will always have something - even if it's low resolution - to render any part of the tileset that becomes visible.\nWhen false, overall loading will be faster, but newly-visible parts of the tileset may initially be blank.";
constexpr const char* GENERATE_MISSING_NORMALS_DESC = "Whether to generate smooth normals when normals are missing in theoriginal Gltf.\n\nAccording to the Gltf spec: \"When normals are not specified, clientimplementations should calculate flat normals.\"\nHowever, calculating flatnormals requires duplicating vertices.\nThis option allows the gltfs to besent with explicit smooth normals when the original gltf was missingnormals.";


/**
* @brief This will be the underlying config for the tileset
* the class basically acts as a builder wrapper to provide
* UI serialization in-engine
*/
class OpaqueTilesetOptions {
public:
	OpaqueTilesetOptions() = default;
	~OpaqueTilesetOptions() = default;
	Cesium3DTilesSelection::TilesetOptions options{};

	Cesium3DTilesSelection::TilesetContentOptions contentOptions{};
};

inline void extract_properties_from_bounding_box(const CesiumGeometry::OrientedBoundingBox& box, Dictionary* refProperties, const CesiumGeoreference* georeference) {
	const Transform3D& ecefToEngineXform = georeference->get_tx_ecef_to_engine();

	Vector3 size = CesiumMathUtils::from_glm_vec3(box.getLengths());
	Vector3 center = CesiumMathUtils::from_glm_vec3(box.getCenter());
	const glm::mat3& halfAxes = box.getHalfAxes();

	const glm::dvec3& basisX = halfAxes[0];
	const glm::dvec3& basisY = halfAxes[1];
	const glm::dvec3& basisZ = halfAxes[2];
	Basis basisLocal = Basis(
		CesiumMathUtils::from_glm_vec3(basisX),
		CesiumMathUtils::from_glm_vec3(basisY),
		CesiumMathUtils::from_glm_vec3(basisZ)
	).orthonormalized();

	Transform3D xform = ecefToEngineXform * Transform3D(basisLocal, center);

	// Check if we are georeferenced with cartographic origin and substract the origin if so
	if (georeference->get_origin_type() == static_cast<int32_t>(CesiumGeoreference::OriginType::CartographicOrigin)) {
		const Vector3 relativeOrigin = ecefToEngineXform.xform(CesiumMathUtils::from_glm_vec3(georeference->get_ecef_position()));
		xform.set_origin(xform.get_origin() - relativeOrigin);
	}

	if( !refProperties->has("size") ) {
		(*refProperties)["size"] = size;
	}
	if( !refProperties->has("transform") ) {
		(*refProperties)["transform"] = xform;
	}
}

inline void draw_debug_volume_from_variant(const Cesium3DTilesSelection::BoundingVolume& boundingVariant, const Callable& callback, const CesiumGeoreference* georeference) {
	// Determine the type of bounding volume
	size_t typeIndex = boundingVariant.index();
	EBoundingType boundingType = static_cast<EBoundingType>(typeIndex + 1);
	Dictionary properties{};

	// Extract our rotation here
	const Transform3D& ecefToEngineXform = georeference->get_tx_ecef_to_engine();

	switch (boundingType) {
        case EBoundingType::Sphere:
        	{
	    		const auto& sphere = std::get<CesiumGeometry::BoundingSphere>(boundingVariant);
				Vector3 center = ecefToEngineXform.xform(CesiumMathUtils::from_glm_vec3(sphere.getCenter()));
				if (georeference->get_origin_type() == static_cast<int32_t>(CesiumGeoreference::OriginType::CartographicOrigin)) {
					const Vector3 relativeOrigin = ecefToEngineXform.xform(CesiumMathUtils::from_glm_vec3(georeference->get_ecef_position()));
					center -= relativeOrigin;
				}
				if( !properties.has("center") ){
					properties["center"] = center;
				}
				if( !properties.has("radius") ){
					properties["radius"] = sphere.getRadius();
				}
			}
			break;
        case EBoundingType::Box:
        	{
	        	const auto& box = std::get<CesiumGeometry::OrientedBoundingBox>(boundingVariant);
				extract_properties_from_bounding_box(box, &properties, georeference);
        	}
			break;
        case EBoundingType::RegionWithLooseFittingHeights:
        	{
        		const auto& boundingRegionLoose = std::get<CesiumGeospatial::BoundingRegionWithLooseFittingHeights>(boundingVariant);
        		const CesiumGeometry::OrientedBoundingBox& box = boundingRegionLoose.getBoundingRegion().getBoundingBox();
        		extract_properties_from_bounding_box(box, &properties, georeference);
        	}
        case EBoundingType::Region:
	        	{
		        	const auto& region = std::get<CesiumGeospatial::BoundingRegion>(boundingVariant);
		        	const CesiumGeometry::OrientedBoundingBox& box = region.getBoundingBox();
		        	extract_properties_from_bounding_box(box, &properties, georeference);
	        	}
        	break;
        case EBoundingType::CellVolume:
        case EBoundingType::CylinderRegion:
      	default:
      		{
				ERR_PRINT("NOT YET IMPLEMENTED");
      		}
        	break;
      		// Not identified, send in debug
    }
	
	// Call the GDScript function
	godot::Array callback_args;
	callback_args.push_back(static_cast<int32_t>(boundingType));
	callback_args.push_back(properties);
	callback.callv(callback_args);
}

Cesium3DTileset::Cesium3DTileset()
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


Cesium3DTileset::~Cesium3DTileset() {
	// Trivial, but GCC complains if this is not in the implementation file
}

void Cesium3DTileset::set_maximum_screen_space_error(real_t error)
{
	this->m_tilesetConfig->options.maximumScreenSpaceError = error;
}

real_t Cesium3DTileset::get_maximum_screen_space_error() const
{
	return this->m_tilesetConfig->options.maximumScreenSpaceError;
}

void Cesium3DTileset::set_maximum_simultaneous_tile_loads(uint32_t count)
{
	this->m_tilesetConfig->options.maximumSimultaneousTileLoads = count;
}

uint32_t Cesium3DTileset::get_maximum_simultaneous_tile_loads() const
{
	return this->m_tilesetConfig->options.maximumSimultaneousTileLoads;
}

void Cesium3DTileset::set_preload_ancestors(bool preload)
{
	this->m_tilesetConfig->options.preloadAncestors = preload;
}

bool Cesium3DTileset::get_preload_ancestors() const
{
	return this->m_tilesetConfig->options.preloadAncestors;
}

void Cesium3DTileset::set_preload_siblings(bool preload)
{
	this->m_tilesetConfig->options.preloadSiblings = preload;
}

bool Cesium3DTileset::get_preload_siblings() const
{
	return this->m_tilesetConfig->options.preloadSiblings;
}

void Cesium3DTileset::set_loading_descendant_limit(uint32_t limit)
{
	this->m_tilesetConfig->options.loadingDescendantLimit = limit;
}

uint32_t Cesium3DTileset::get_loading_descendant_limit() const
{
	return this->m_tilesetConfig->options.loadingDescendantLimit;
}

void Cesium3DTileset::set_forbid_holes(bool forbidHoles)
{
	this->m_tilesetConfig->options.forbidHoles = forbidHoles;
}

bool Cesium3DTileset::get_forbid_holes() const
{
	return this->m_tilesetConfig->options.forbidHoles;
}

void Cesium3DTileset::set_url(const String& url)
{
	//Assert the source
	this->m_url = url;
	this->recreate_tileset();
}

const String& Cesium3DTileset::get_url() const
{
	return this->m_url;
}

void Cesium3DTileset::set_generate_missing_normals_smooth(bool shouldGenerate)
{
	this->m_tilesetConfig->contentOptions.generateMissingNormalsSmooth = shouldGenerate;
}


bool Cesium3DTileset::get_generate_missing_normals_smooth() const
{
	return this->m_tilesetConfig->contentOptions.generateMissingNormalsSmooth;
}

int Cesium3DTileset::get_data_source() const
{
	return static_cast<int>(this->m_selectedDataSource);
}

void Cesium3DTileset::set_data_source(int data_source)
{
	this->m_selectedDataSource = static_cast<CesiumDataSource>(data_source);
	this->notify_property_list_changed();
}

void Cesium3DTileset::set_ion_asset_id(int64_t id)
{
	this->m_cesiumIonAssetId = id;
}

int64_t Cesium3DTileset::get_ion_asset_id() const
{
	return this->m_cesiumIonAssetId;
}

void Cesium3DTileset::set_create_physics_meshes(bool shouldCreate)
{
	this->m_createPhysicsMeshes = shouldCreate;
}

bool Cesium3DTileset::get_create_physics_meshes() const
{
	return this->m_createPhysicsMeshes;
}

void Cesium3DTileset::update_tileset(const Transform3D& cameraTransform)
{
	
	bool isGeoreferenced = this->is_georeferenced(&this->m_georeference);
	if (this->m_activeTileset == nullptr) {
		if (isGeoreferenced) {
			this->m_georeference->set_should_update_origin(true);
			this->m_georeference->register_tileset_to_move_origin(this);
		}
		
		this->load_tileset();
	}

	//Get the camera view state
	Viewport* currentViewport = get_viewport();
	Camera3D* cam = currentViewport->get_camera_3d();

	glm::dvec3 camPos;
	if (isGeoreferenced) {
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

	double verticalFOV = Math::deg_to_rad(cam->get_fov());
	double horizontalFOV = 2 * Math::atan(aspectRatioF * Math::tan(verticalFOV * 0.5));
	Cesium3DTilesSelection::ViewState currentViewState = Cesium3DTilesSelection::ViewState(
		camPos,
		cameraDirection,
		CesiumMathUtils::to_glm_dvec3(cameraUp),
		CesiumMathUtils::to_glm_vec2(viewportSize),
		horizontalFOV,
		verticalFOV
	);

	const Cesium3DTilesSelection::ViewUpdateResult& updateResult = this->m_activeTileset->updateViewGroup(this->m_activeTileset->getDefaultViewGroup(), { currentViewState });
	this->m_activeTileset->loadTiles();

	for (CesiumUtility::IntrusivePointer<const Cesium3DTilesSelection::Tile> tile : updateResult.tilesToRenderThisFrame) {
		this->render_tile_as_node(*tile);
	}
	for (CesiumUtility::IntrusivePointer<const Cesium3DTilesSelection::Tile> tile : updateResult.tilesFadingOut) {
		despawn_tile(*tile);
	}
}

void Cesium3DTileset::set_debug_boundig_volumes_func(const Callable& onTileDrawn) {
	this->m_debugVolumesFunction = onTileDrawn;
}

bool Cesium3DTileset::is_initial_loading_finished() const
{
	return this->m_initialLoadingFinished;
}

void Cesium3DTileset::add_overlay(CesiumIonRasterOverlay* overlay)
{
	if (overlay == nullptr) return;
	this->m_activeTileset->getOverlays().add(overlay->get_overlay_instance());
}

void Cesium3DTileset::free_tile(Cesium3DTile* tileInstance, uint64_t tileHash) {
	if (tileInstance == nullptr) {
		return;
	}
	// Mark for deletion and empty the slot on the hash map
	tileInstance->queue_free();
}

bool Cesium3DTileset::is_georeferenced(CesiumGeoreference** outRef) const
{
	if (this->m_georeference != nullptr) {
		*outRef = this->m_georeference;
		return this->m_georeference->get_origin_type() == static_cast<int32_t>(CesiumGeoreference::OriginType::CartographicOrigin);
	}
	//Check if the parent is of type CesiumGDGeoreference
	Node3D* parent = this->get_parent_node_3d();
	*outRef = Object::cast_to<CesiumGeoreference>(parent);
	return (*outRef)->get_origin_type() == static_cast<int32_t>(CesiumGeoreference::OriginType::CartographicOrigin);
}


void Cesium3DTileset::move_origin(const glm::dvec3& enginePos) {
	// On true origin, this is a no-op, so, we better save those CPU cycles
	if (this->m_georeference->get_origin_type_raw() == CesiumGeoreference::OriginType::TrueOrigin) {
		return;
	}
	// Get all tiles
	int32_t childCount = this->get_child_count();
	for(int32_t i = 0; i < childCount; i++) {
		// This applies to user meshes and 3D tiles alike
		GeoreferencedMesh* currMesh = Object::cast_to<GeoreferencedMesh>(this->get_child(i));
		if (currMesh == nullptr) {
			continue;
		}
		currMesh->apply_position_on_globe(enginePos);
	}
}

void Cesium3DTileset::recreate_tileset()
{
	// NYI: Implementation of editor level rendering is pending
}

void Cesium3DTileset::load_tileset()
{	
	//Get the options to read the tileset and then load it into memory
	const Cesium3DTilesSelection::TilesetOptions& options = this->m_tilesetConfig->options;
	const Cesium3DTilesSelection::TilesetContentOptions& contentOptions = this->m_tilesetConfig->contentOptions;

	if (this->m_selectedDataSource == CesiumDataSource::FromCesiumIon) {
		const String& token = CesiumGDConfig::get_singleton(this)->get_access_token();
		this->m_activeTileset = std::make_unique<Cesium3DTilesSelection::Tileset>(
			this->create_tileset_externals(),
			this->m_cesiumIonAssetId,
			token.utf8().get_data(),
			options
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
		CesiumIonRasterOverlay* overlay = Object::cast_to<CesiumIonRasterOverlay>(currChild);
		if (overlay == nullptr) continue;
		overlay->add_to_tileset(this);
	}

}

Cesium3DTilesSelection::TilesetExternals Cesium3DTileset::create_tileset_externals()
{
	const String cachePath = "user://cache";
	Ref<DirAccess> userAccess = DirAccess::open("user://");
	Error err = userAccess->make_dir_recursive(cachePath);
	if (err != Error::OK) {
		ERR_PRINT("Could not create / use temporary cache path!");
	}
	String globalCachePath = ProjectSettings::get_singleton()->globalize_path(cachePath) + "/cesium-request-cache.sqlite";
	constexpr int32_t requestsPerCachePrune = 10000;
	constexpr uint64_t maxItems = 4096 * 5;

	auto cache = std::make_shared<CesiumAsync::SqliteCache>(spdlog::default_logger(), globalCachePath.utf8().get_data(), maxItems);
	auto simpleAccessor = std::make_shared<NetworkAssetAccessor>();
	auto cachedAccessor = std::make_shared<CesiumAsync::CachingAssetAccessor>(spdlog::default_logger(), simpleAccessor, cache, requestsPerCachePrune);
	auto gunzipAccessor = std::make_shared<CesiumAsync::GunzipAssetAccessor>(
		cachedAccessor
	);
	
	auto taskProcessor = std::make_shared<SimpleTaskProcessor>();
	CesiumAsync::AsyncSystem asyncSystem(taskProcessor);
	auto renderResourcesProvider = std::make_shared<GodotPrepareRenderResources>(this);
	auto creditSystem = std::make_shared<CesiumUtility::CreditSystem>();
	CesiumGDCreditSystem::get_singleton(this)->add_credit_system(creditSystem);
	
	Cesium3DTilesSelection::TilesetExternals result {
		gunzipAccessor,
		renderResourcesProvider,
		asyncSystem,
		creditSystem
	};
	return result;
}

void Cesium3DTileset::render_tile_as_node(const Cesium3DTilesSelection::Tile& tile)
{
	if (tile.getState() == Cesium3DTilesSelection::TileLoadState::Failed) {
		std::string tileIdStr = Cesium3DTilesSelection::TileIdUtilities::createTileIdString(tile.getTileID());
		ERR_PRINT(String("Failed to load tile ") + tileIdStr.c_str());
		return;
	}
	
	if (tile.getState() != Cesium3DTilesSelection::TileLoadState::Done) {
		return;
	}

	const Cesium3DTilesSelection::TileContent& content = tile.getContent();
	const Cesium3DTilesSelection::TileRenderContent* renderContent = content.getRenderContent();
	
	if (renderContent == nullptr) {
		return;
	}
	Cesium3DTile* foundNode = static_cast<Cesium3DTile*>(renderContent->getRenderResources());
	if (foundNode == nullptr) return;
	
	if (this->m_createPhysicsMeshes) {
		Node* collisionNode = foundNode->get_child_count() < 1 ? nullptr : foundNode->get_child(0);
		if (collisionNode == nullptr) return;
		CollisionShape3D* shape = collisionNode->get_child_count() < 1 ? nullptr : Object::cast_to<CollisionShape3D>(collisionNode->get_child(0));
		if (shape == nullptr) return;
		shape->set_disabled(false);
	}

	if (!foundNode->is_inside_tree()) {
		size_t hash = std::visit(CesiumVariantHash{}, tile.getTileID());
		this->register_tile(foundNode, hash);
		foundNode->set_name(itos(hash));
	}
	
	if (!this->m_debugVolumesFunction.is_null()) {
		// Get the xform for the current tile rotation
		// Basis + Zero pos * ecef_engine_xform()
		draw_debug_volume_from_variant(tile.getBoundingVolume(), this->m_debugVolumesFunction, this->m_georeference);
	}
	foundNode->show();
}

void Cesium3DTileset::despawn_tile(const Cesium3DTilesSelection::Tile& tile)
{
	if (tile.getState() != Cesium3DTilesSelection::TileLoadState::Done) {
		return;
	}
	const Cesium3DTilesSelection::TileRenderContent* renderContent = tile.getContent().getRenderContent();
	if (renderContent == nullptr) return;
	Cesium3DTile* foundNode = static_cast<Cesium3DTile*>(renderContent->getRenderResources());
	if (foundNode == nullptr) return;
	if (!foundNode->is_inside_tree()) return;
	foundNode->hide();
	// Deactivate the collisions
	if (this->m_createPhysicsMeshes) {
		int32_t childCount = foundNode->get_child_count();
		Node* collisionNode = childCount < 1 ? nullptr : foundNode->get_child(0);
		if (collisionNode == nullptr) return;
		CollisionShape3D* shape = collisionNode->get_child_count() < 1 ? nullptr : Object::cast_to<CollisionShape3D>(collisionNode->get_child(0));
		if (shape == nullptr) return;
		shape->set_disabled(true);
	}
}

void Cesium3DTileset::despawn_tile_deferred(const Cesium3DTilesSelection::Tile& tile)
{
}

bool Cesium3DTileset::try_get_tile_from_instance_id(const ObjectID& objectId, Cesium3DTile** outNode)
{
	*outNode = Object::cast_to<Cesium3DTile>(ObjectDB::get_instance(objectId));
	return *outNode != nullptr;
}


void Cesium3DTileset::process_tile_chunk(const std::vector<Cesium3DTilesSelection::Tile*> &tilesView, int32_t offset, int32_t size) {
	// NOTE: This function is meant to run on a worker thread
	// Given a certain buffer window for our chunk, we can access the memory and render those tiles individually
	for (int32_t i = offset; i < size; i++)
	{
		this->render_tile_as_node(*tilesView.at(i));
	}
}


void Cesium3DTileset::register_tile(Cesium3DTile *instance, size_t hash) {
	this->add_child(instance, false);
	instance->set_owner(this);
	tileCount++;
	if (this->m_georeference->get_origin_type_raw() == CesiumGeoreference::OriginType::TrueOrigin) {
		return;
	}
	const glm::dvec3& ecefOrigin = this->m_georeference->get_ecef_position();
	const glm::dvec3 engineOrigin = CesiumMathUtils::ecef_to_engine(ecefOrigin);
	instance->apply_position_on_globe(engineOrigin);
}


bool Cesium3DTileset::get_show_hierarchy() const {
	return this->m_showHierarchy;
}

void Cesium3DTileset::set_show_hierarchy(bool show) {
	this->m_showHierarchy = show;
}

void Cesium3DTileset::_bind_methods()
{
#pragma region Inspector properties
	ClassDB::bind_method(D_METHOD("set_maximum_screen_space_error", "error"), &Cesium3DTileset::set_maximum_screen_space_error);
	ClassDB::bind_method(D_METHOD("get_maximum_screen_space_error"), &Cesium3DTileset::get_maximum_screen_space_error);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "maximum_screen_space_error", PROPERTY_HINT_NONE, MAXIMUM_SCREEN_SPACE_DESC), "set_maximum_screen_space_error", "get_maximum_screen_space_error");

	
	ClassDB::bind_method(D_METHOD("set_maximum_simultaneous_tile_loads", "count"), &Cesium3DTileset::set_maximum_simultaneous_tile_loads);
	ClassDB::bind_method(D_METHOD("get_maximum_simultaneous_tile_loads"), &Cesium3DTileset::get_maximum_simultaneous_tile_loads);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "maximum_simultaneous_tile_loads", PROPERTY_HINT_NONE, MAXIMUM_SIMULTANEOUS_TILE_LOADS_DESC), "set_maximum_simultaneous_tile_loads", "get_maximum_simultaneous_tile_loads");

	ClassDB::bind_method(D_METHOD("set_preload_ancestors", "preload"), &Cesium3DTileset::set_preload_ancestors);
	ClassDB::bind_method(D_METHOD("get_preload_ancestors"), &Cesium3DTileset::get_preload_ancestors);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preload_ancestors", PROPERTY_HINT_NONE, PRELOAD_ANCESTORS_DESC), "set_preload_ancestors", "get_preload_ancestors");

	ClassDB::bind_method(D_METHOD("set_preload_siblings", "preload"), &Cesium3DTileset::set_preload_siblings);
	ClassDB::bind_method(D_METHOD("get_preload_siblings"), &Cesium3DTileset::get_preload_siblings);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preload_siblings", PROPERTY_HINT_NONE, PRELOAD_SIBLINGS_DESC), "set_preload_siblings", "get_preload_siblings");

	ClassDB::bind_method(D_METHOD("set_loading_descendant_limit", "limit"), &Cesium3DTileset::set_loading_descendant_limit);
	ClassDB::bind_method(D_METHOD("get_loading_descendant_limit"), &Cesium3DTileset::get_loading_descendant_limit);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "loading_descendant_limit", PROPERTY_HINT_NONE, LOADING_DESCENDANT_LIMIT_DESC), "set_loading_descendant_limit", "get_loading_descendant_limit");

	ClassDB::bind_method(D_METHOD("set_forbid_holes", "forbidHoles"), &Cesium3DTileset::set_forbid_holes);
	ClassDB::bind_method(D_METHOD("get_forbid_holes"), &Cesium3DTileset::get_forbid_holes);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "forbid_holes", PROPERTY_HINT_NONE, FORBID_HOLES_DESC), "set_forbid_holes", "get_forbid_holes");

	ClassDB::bind_method(D_METHOD("set_generate_missing_normals_smooth", "shouldGenerate"), &Cesium3DTileset::set_generate_missing_normals_smooth);
	ClassDB::bind_method(D_METHOD("get_generate_missing_normals_smooth"), &Cesium3DTileset::get_generate_missing_normals_smooth);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "generate_missing_normals_smooth", PROPERTY_HINT_NONE, GENERATE_MISSING_NORMALS_DESC), "set_generate_missing_normals_smooth", "get_generate_missing_normals_smooth");

	ClassDB::bind_method(D_METHOD("set_create_physics_meshes", "shouldGenerate"), &Cesium3DTileset::set_create_physics_meshes);
	ClassDB::bind_method(D_METHOD("get_create_physics_meshes"), &Cesium3DTileset::get_create_physics_meshes);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "create_physics_meshes"), "set_create_physics_meshes", "get_create_physics_meshes");

	ClassDB::bind_method(D_METHOD("get_data_source"), &Cesium3DTileset::get_data_source);
	ClassDB::bind_method(D_METHOD("set_data_source", "data_source"), &Cesium3DTileset::set_data_source);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "data_source", PROPERTY_HINT_ENUM, "From Cesium Ion,From Url"), "set_data_source", "get_data_source");
	ClassDB::bind_integer_constant(get_class_static(), "CesiumDataSource", "FromCesiumIon", static_cast<int32_t>(CesiumDataSource::FromCesiumIon));
	ClassDB::bind_integer_constant(get_class_static(), "CesiumDataSource", "FromUrl", static_cast<int32_t>(CesiumDataSource::FromUrl));
	

	ClassDB::bind_method(D_METHOD("set_url", URL_P_NAME), &Cesium3DTileset::set_url);
	
	ClassDB::bind_method(D_METHOD("get_url"), &Cesium3DTileset::get_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "url"), "set_url", "get_url");

	ClassDB::bind_method(D_METHOD("set_ion_asset_id", ION_ASSET_ID_P_NAME), &Cesium3DTileset::set_ion_asset_id);
	ClassDB::bind_method(D_METHOD("get_ion_asset_id"), &Cesium3DTileset::get_ion_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ion_asset_id"), "set_ion_asset_id", "get_ion_asset_id");


	ClassDB::bind_method(D_METHOD("set_show_hierarchy", "showHierarchy"), &Cesium3DTileset::set_show_hierarchy);
	ClassDB::bind_method(D_METHOD("get_show_hierarchy"), &Cesium3DTileset::get_show_hierarchy);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_hierarchy"), "set_show_hierarchy", "get_show_hierarchy");
	
#pragma endregion

#pragma region Public methods
	ClassDB::bind_method(D_METHOD("is_initial_loading_finished"), &Cesium3DTileset::is_initial_loading_finished);
	ClassDB::bind_method(D_METHOD("update_tileset", "camera_transform"), &Cesium3DTileset::update_tileset);
	ClassDB::bind_method(D_METHOD("set_debug_bounding_volumes_func", "onTileDrawn"), &Cesium3DTileset::set_debug_boundig_volumes_func);
	ClassDB::bind_method(D_METHOD("free_tile"), &Cesium3DTileset::free_tile);
#pragma endregion

	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "Box", static_cast<int32_t>(EBoundingType::Box));
	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "CellVolume", static_cast<int32_t>(EBoundingType::CellVolume));
	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "CylinderRegion", static_cast<int32_t>(EBoundingType::CylinderRegion));
	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "Region", static_cast<int32_t>(EBoundingType::Region));
	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "RegionWithLooseFittingHeights", static_cast<int32_t>(EBoundingType::RegionWithLooseFittingHeights));
	ClassDB::bind_integer_constant(get_class_static(), "BoundingType", "Sphere", static_cast<int32_t>(EBoundingType::Sphere));

}

void Cesium3DTileset::_get_property_list(List<PropertyInfo>* properties) const
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


uint32_t Cesium3DTileset::update_property_usage_flags(const PropertyInfo& propertyRef) const
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

bool Cesium3DTileset::_set(const StringName& p_name, const Variant& p_property)
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

bool Cesium3DTileset::_get(const StringName& p_name, Variant& r_property) const
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


void Cesium3DTileset::_ready() {
	if (!is_editor_mode()) return;
	Node* root = this->get_tree()->get_root();
	Camera3D* foundCamera = Godot3DTiles::AssetManipulation::find_georef_cam(root);
	if (foundCamera == nullptr) {
		WARN_PRINT("Could not find a Cesium Dynamic camera, try adding it manually in the Cesium Ion Panel");
		return;
	}
	Godot3DTiles::AssetManipulation::update_camera_tilesets(foundCamera);
}


CesiumGeoreference* Cesium3DTileset::get_georeference_node() const {
	return this->m_georeference;
}

void Cesium3DTileset::_enter_tree() {
	if (!is_editor_mode()) {
		// TODO: Replace this, it's bad code lol
		this->is_georeferenced(&this->m_georeference);
		return;
	}
	CesiumGeoreference* globe = Godot3DTiles::AssetManipulation::find_or_create_globe(this);
	if (globe == nullptr) {
		return;
	}
	//Parent to the globe
	this->m_georeference = globe;
	this->reparent(globe, true);
	this->set_rotation_degrees(Vector3(90.0, 0.0, 0.0));
	this->set_owner(globe->get_parent_node_3d());
}

