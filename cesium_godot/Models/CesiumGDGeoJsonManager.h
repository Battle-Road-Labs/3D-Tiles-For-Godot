#ifndef CESIUM_GD_GEOJSON_MANAGER_H
#define CESIUM_GD_GEOJSON_MANAGER_H

#if defined(CESIUM_GD_EXT)
#   include <godot_cpp/classes/node.hpp>
#   include <godot_cpp/classes/http_request.hpp>
#   include <godot_cpp/variant/array.hpp>
#   include <godot_cpp/variant/dictionary.hpp>
#   include <godot_cpp/variant/string.hpp>
#   include <godot_cpp/variant/packed_string_array.hpp>
#   include <godot_cpp/variant/packed_byte_array.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#   include "scene/main/node.h"
#   include "scene/main/http_request.h"
#endif

#include "CesiumGDGeoJsonLoader.h"

class CesiumGDGeoJsonManager : public Node {
    GDCLASS(CesiumGDGeoJsonManager, Node)

public:
    CesiumGDGeoJsonManager();
    ~CesiumGDGeoJsonManager() override = default;

    void _ready() override;

    String assets_search;
    void set_assets_search(String v) { assets_search = v; }
    String get_assets_search()  const { return assets_search; }

    void set_auto_load_geojsons(bool v) { m_autoLoadGeojsons = v; }
    bool get_auto_load_geojsons() const { return m_autoLoadGeojsons; }

    void set_fetsch_geojsons_at_startup(bool v) { m_fetchGeojsonsAtStartup = v; notify_property_list_changed(); }
    bool get_fetsch_geojsons_at_startup() const { return m_fetchGeojsonsAtStartup; }

    Array assetData;
    void set_assetData(Array v) { assetData = v; }
    Array get_assetData() const { return assetData; }

    Array geojsonLoaders;
    void set_geojsonLoaders(Array v) { geojsonLoaders = v; }
    Array get_geojsonLoaders() const { return geojsonLoaders; }

    void fetch_assets();

    Array get_asset_infos() const { return assetData; }

    Array get_loaders() const { return geojsonLoaders; }

    CesiumGDGeoJsonLoader* get_loader_by_id(int64_t asset_id) const;

    void load_all();

    void _on_assets_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);

protected:
    static void _bind_methods();

    void _get_property_list(List<PropertyInfo>* p_list) const;
    bool _get(const StringName& p_name, Variant& r_property) const;
    bool _set(const StringName& p_name, const Variant& p_property);

private:
    bool m_autoLoadGeojsons = false;
    bool m_fetchGeojsonsAtStartup = false;

    HTTPRequest* m_assetsRequest = nullptr;

    void _create_loaders();
    void _emit_failure(const String& msg);
};

#endif // CESIUM_GD_GEOJSON_MANAGER_H