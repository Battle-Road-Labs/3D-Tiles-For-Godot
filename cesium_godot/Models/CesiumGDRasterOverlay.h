#ifndef CESIUM_GD_RASTER_OVERLAY_H
#define CESIUM_GD_RASTER_OVERLAY_H

//Make Cesium not check for thread safety
#define NDEBUG


#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

#include <CesiumUtility/IntrusivePointer.h>
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include <CesiumRasterOverlays/TileMapServiceRasterOverlay.h>
#include "CesiumDataSource.h"

class Cesium3DTileset;

class CesiumGDConfig;

class CesiumIonRasterOverlay : public Node3D {
	GDCLASS(CesiumIonRasterOverlay, Node3D)
public:
#pragma region Editor Properties
	int64_t get_asset_id() const;

	void set_asset_id(int64_t id);

	void set_material_key(const String& key);

	const String& get_material_key() const;

	void set_url(const String& url);

	const String& get_url() const;

	int get_data_source() const;

	void set_data_source(int data_source);

#pragma endregion

	Error add_to_tileset(Cesium3DTileset* tilesetInstance);

	void remove_from_tileset(Cesium3DTileset* tilesetInstance);

	CesiumUtility::IntrusivePointer<CesiumRasterOverlays::RasterOverlay> get_overlay_instance();

private:


	void create_and_add_overlay(Cesium3DTileset* tilesetInstance);

	int64_t m_assetId = 0;

	String m_materialKey = "overlay";

	String m_url = "";

	CesiumDataSource m_selectedDataSource = CesiumDataSource::FromCesiumIon;

	CesiumUtility::IntrusivePointer<CesiumRasterOverlays::RasterOverlay> m_overlayInstance;


protected:
	static void _bind_methods();

	void _get_property_list(List<PropertyInfo>* properties) const;

	bool _set(const StringName& p_name, const Variant& p_property);
	bool _get(const StringName& p_name, Variant& r_property) const;
};

#endif // !CESIUM_GD_RASTER_OVERLAY_H

