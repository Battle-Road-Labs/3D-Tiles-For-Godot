#ifndef CESIUM_GD_GEOJSON_LOADER_H
#define CESIUM_GD_GEOJSON_LOADER_H

#if defined(CESIUM_GD_EXT)
#   include <godot_cpp/classes/node.hpp>
#   include <godot_cpp/classes/http_request.hpp>
#   include <godot_cpp/variant/dictionary.hpp>
#   include <godot_cpp/variant/array.hpp>
#   include <godot_cpp/variant/string.hpp>
#   include <godot_cpp/variant/packed_string_array.hpp>
#   include <godot_cpp/variant/packed_byte_array.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#   include "scene/main/node.h"
#   include "scene/main/http_request.h"
#endif

#include <string>

class CesiumGDGeoJsonLoader : public Node {
    GDCLASS(CesiumGDGeoJsonLoader, Node)

public:
    CesiumGDGeoJsonLoader();
    ~CesiumGDGeoJsonLoader() override = default;

    void _ready() override;

    void set_ion_asset_id(int64_t id) { m_ionAssetId = id; }
    int64_t get_ion_asset_id() const { return m_ionAssetId; }

    void set_use_manual_token(bool v) { m_useManualToken = v; }
    bool get_use_manual_token() const { return m_useManualToken; }

    void set_ion_access_token(const String& t) { m_ionAccessToken = t; }
    String get_ion_access_token() const { return m_ionAccessToken; }

    void set_auto_load(bool v) { m_autoLoad = v; }
    bool get_auto_load() const { return m_autoLoad; }

    void load();
    Array get_features() const { return m_features; }

    enum LoadState { IDLE = 0, LOADING = 1, READY = 2, FAILED = 3 };
    LoadState get_load_state() const { return m_state; }

    void _on_endpoint_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);

    void _on_geojson_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);

protected:
    static void _bind_methods();

private:
    int64_t m_ionAssetId = 0;
    bool m_useManualToken = false;
    String m_ionAccessToken;
    bool m_autoLoad = false;

    LoadState m_state = IDLE;
    Array m_features;

    HTTPRequest* m_endpointRequest = nullptr;
    HTTPRequest* m_geojsonRequest = nullptr;

    void _request_endpoint();
    void _request_geojson(const String& url, const String& temp_token);
    bool _parse_geojson(const PackedByteArray& body);

    void _emit_success();
    void _emit_failure(const String& msg);

    HTTPRequest* _make_http_node(const String& name, const float& p_timeout = 0.0);
};

VARIANT_ENUM_CAST(CesiumGDGeoJsonLoader::LoadState);

#endif // CESIUM_GD_GEOJSON_LOADER_H