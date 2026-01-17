#ifndef CESIUM_GD_TILESET_H
#define CESIUM_GD_TILESET_H

#if defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#elif defined(CESIUM_GD_EXT)
#include "godot_cpp/core/property_info.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/callable.hpp>
using namespace godot;
#endif

#include "Cesium3DTilesSelection/Tileset.h"

#include "CesiumDataSource.h"
#include "../Utils/BRThreadPool.h"

namespace Cesium3DTilesSelection {
	class Tileset;
	class Tile;
	class TilesetExternals;
}

class OpaqueTilesetOptions;


class CesiumIonRasterOverlay;

class CesiumGeoreference;

class Cesium3DTile;

enum class EBoundingType {
	None,
	Sphere,
	Box,
	Region,
	RegionWithLooseFittingHeights,
	CellVolume,
	CylinderRegion
};

class Cesium3DTileset : public Node3D
{
	GDCLASS(Cesium3DTileset, Node3D)

public:
	Cesium3DTileset();
	~Cesium3DTileset();
#pragma region Public Editor Methods

	void set_maximum_screen_space_error(real_t error);

	real_t get_maximum_screen_space_error() const;
	
	void set_maximum_simultaneous_tile_loads(uint32_t count);

	uint32_t get_maximum_simultaneous_tile_loads() const;

	void set_preload_ancestors(bool preload);

	bool get_preload_ancestors() const;

	void set_preload_siblings(bool preload);

	bool get_preload_siblings() const;

	void set_loading_descendant_limit(uint32_t limit);

	uint32_t get_loading_descendant_limit() const;

	void set_forbid_holes(bool forbidHoles);

	bool get_forbid_holes() const;

	void set_url(const String& url);

	const String& get_url() const;

	void set_generate_missing_normals_smooth(bool shouldGenerate);

	bool get_generate_missing_normals_smooth() const;

	int get_data_source() const;

	void set_data_source(int data_source);

	void set_ion_asset_id(int64_t id);

	int64_t get_ion_asset_id() const;
	
	void set_create_physics_meshes(bool shouldCreate);

	bool get_create_physics_meshes() const;

	bool get_show_hierarchy() const;

	void set_show_hierarchy(bool show);

#pragma endregion

	void update_tileset(const Transform3D& cameraTransform);

	void set_debug_boundig_volumes_func(const Callable& onTileDrawn);
	
	bool is_initial_loading_finished() const;

	void add_overlay(CesiumIonRasterOverlay* overlay);

	void free_tile(Cesium3DTile* tileInstance, size_t tileHash);
	
	bool is_georeferenced(CesiumGeoreference** outRef) const;

	void move_origin(const glm::dvec3& enginePos);

	void set_for_each_tile_func(const Callable& onTileFunc);

	CesiumGeoreference* get_georeference_node() const;
	
	void _enter_tree() override;

	void _ready() override;

private:

	void recreate_tileset();

	void load_tileset();

	Cesium3DTilesSelection::TilesetExternals create_tileset_externals();

	void render_tile_as_node(const Cesium3DTilesSelection::Tile& tile);

	void despawn_tile(const Cesium3DTilesSelection::Tile& tile);

	void despawn_tile_deferred(const Cesium3DTilesSelection::Tile& tile);

	bool try_get_tile_from_instance_id(const ObjectID& objectId, Cesium3DTile** outNode);

	void process_tile_chunk(const std::vector<Cesium3DTilesSelection::Tile*>& tilesView, int32_t offset, int32_t size);

	void register_tile(Cesium3DTile *instance, size_t hash);

	uint32_t update_property_usage_flags(const PropertyInfo& property) const;
	
	std::unique_ptr<Cesium3DTilesSelection::Tileset> m_activeTileset = nullptr;


	OpaqueTilesetOptions* m_tilesetConfig;

	bool m_createPhysicsMeshes = true;

	String m_url{};

	int64_t m_cesiumIonAssetId = 0;

	bool m_initialLoadingFinished;

	bool m_showHierarchy;

	CesiumDataSource m_selectedDataSource = CesiumDataSource::FromCesiumIon;

	CesiumGeoreference* m_georeference = nullptr;

	BRThreadPool m_signalingThreadPool;

	Callable m_debugVolumesFunction;

	Callable m_forEachTileFunction;
protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo>* properties) const;

	bool _set(const StringName& p_name, const Variant& p_property);
	bool _get(const StringName& p_name, Variant& r_property) const;
};

#endif
