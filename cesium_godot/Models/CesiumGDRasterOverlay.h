#ifndef CESIUM_GD_RASTER_OVERLAY_H
#define CESIUM_GD_RASTER_OVERLAY_H

//Make Cesium not check for thread safety
#ifndef NDEBUG
#define NDEBUG
#endif


#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

#include <CesiumUtility/IntrusivePointer.h>
#include <CesiumRasterOverlays/IonRasterOverlay.h>

class CesiumGDTileset;

class CesiumGDConfig;

class CesiumGDRasterOverlay : public Node3D {
	GDCLASS(CesiumGDRasterOverlay, Node3D)
public:
#pragma region Editor Properties
	int64_t get_asset_id() const;

	void set_asset_id(int64_t id);

	void set_material_key(const String& key);

	const String& get_material_key() const;

#pragma endregion

	Error add_to_tileset(CesiumGDTileset* tilesetInstance);

	void remove_from_tileset(CesiumGDTileset* tilesetInstance);

	CesiumUtility::IntrusivePointer<CesiumRasterOverlays::IonRasterOverlay> get_overlay_instance();

private:


	void create_and_add_overlay(CesiumGDTileset* tilesetInstance);

	int64_t m_assetId = 0;

	String m_materialKey = "0";

	Ref<CesiumGDConfig> m_configInstance;

	CesiumUtility::IntrusivePointer<CesiumRasterOverlays::IonRasterOverlay> m_overlayInstance;

protected:
	static void _bind_methods();
};

#endif // !CESIUM_GD_RASTER_OVERLAY_H

