#include "CesiumGDGeoJsonLoader.h"
#include "CesiumGDConfig.h"

#if defined(CESIUM_GD_EXT)
#   include <godot_cpp/core/class_db.hpp>
#   include <godot_cpp/variant/utility_functions.hpp>
#   include <godot_cpp/classes/json.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#   include "core/object/class_db.h"
#   include "core/io/json.h"
#endif

CesiumGDGeoJsonLoader::CesiumGDGeoJsonLoader() {}

void CesiumGDGeoJsonLoader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_ion_asset_id", "id"), &CesiumGDGeoJsonLoader::set_ion_asset_id);
    ClassDB::bind_method(D_METHOD("get_ion_asset_id"), &CesiumGDGeoJsonLoader::get_ion_asset_id);
    ClassDB::bind_method(D_METHOD("set_use_manual_token", "enabled"), &CesiumGDGeoJsonLoader::set_use_manual_token);
    ClassDB::bind_method(D_METHOD("get_use_manual_token"), &CesiumGDGeoJsonLoader::get_use_manual_token);
    ClassDB::bind_method(D_METHOD("set_ion_access_token", "token"), &CesiumGDGeoJsonLoader::set_ion_access_token);
    ClassDB::bind_method(D_METHOD("get_ion_access_token"), &CesiumGDGeoJsonLoader::get_ion_access_token);
    ClassDB::bind_method(D_METHOD("set_auto_load", "enabled"), &CesiumGDGeoJsonLoader::set_auto_load);
    ClassDB::bind_method(D_METHOD("get_auto_load"), &CesiumGDGeoJsonLoader::get_auto_load);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "ion_asset_id"), "set_ion_asset_id", "get_ion_asset_id");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_manual_token"), "set_use_manual_token", "get_use_manual_token");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "ion_access_token", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NO_EDITOR), "set_ion_access_token", "get_ion_access_token");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_load"), "set_auto_load", "get_auto_load");

    ClassDB::bind_method(D_METHOD("load"), &CesiumGDGeoJsonLoader::load);
    ClassDB::bind_method(D_METHOD("get_features"), &CesiumGDGeoJsonLoader::get_features);
    ClassDB::bind_method(D_METHOD("get_load_state"), &CesiumGDGeoJsonLoader::get_load_state);

    ClassDB::bind_method(D_METHOD("_on_endpoint_completed", "result", "response_code", "headers", "body"), &CesiumGDGeoJsonLoader::_on_endpoint_completed);
    ClassDB::bind_method(D_METHOD("_on_geojson_completed", "result", "response_code", "headers", "body"), &CesiumGDGeoJsonLoader::_on_geojson_completed);

    ADD_SIGNAL(MethodInfo("geojson_loaded", PropertyInfo(Variant::ARRAY, "features")));
    ADD_SIGNAL(MethodInfo("load_failed", PropertyInfo(Variant::STRING, "error")));

    BIND_ENUM_CONSTANT(IDLE);
    BIND_ENUM_CONSTANT(LOADING);
    BIND_ENUM_CONSTANT(READY);
    BIND_ENUM_CONSTANT(FAILED);
}

void CesiumGDGeoJsonLoader::_ready() {
    m_endpointRequest = _make_http_node("EndpointReq", 20.0);
    m_geojsonRequest = _make_http_node("GeoJsonReq");

    m_endpointRequest->connect("request_completed", Callable(this, "_on_endpoint_completed"));
    m_geojsonRequest->connect("request_completed", Callable(this, "_on_geojson_completed"));

    String ionAccessToken = CesiumGDConfig::get_singleton(this)->get_access_token();
    if (m_useManualToken && !m_ionAccessToken.is_empty()) {
        ionAccessToken = m_ionAccessToken;
    }

    if (m_autoLoad && m_ionAssetId > 0 && !ionAccessToken.is_empty()) {
        load();
    }
}

HTTPRequest* CesiumGDGeoJsonLoader::_make_http_node(const String& name, const float &p_timeout) {
    HTTPRequest* req = memnew(HTTPRequest);
    req->set_name(name);
    req->set_use_threads(true);
    req->set_timeout(p_timeout);
    add_child(req);
    return req;
}

void CesiumGDGeoJsonLoader::load() {
    if (m_state == LOADING) {
        UtilityFunctions::push_warning("CesiumGDGeoJsonLoader: loading, ignore this request.");
        return;
    }

    if (m_ionAssetId <= 0) {
        _emit_failure("ion_asset_id is empty");
        return;
    }

    m_state = LOADING;
    m_features.clear();
    _request_endpoint();
}

// GET /v1/assets/{id}/endpoint
void CesiumGDGeoJsonLoader::_request_endpoint() {
    String url = "https://api.cesium.com/v1/assets/" + String::num_int64(m_ionAssetId) + "/endpoint";
    String ionAccessToken = CesiumGDConfig::get_singleton(this)->get_access_token();
    if (m_useManualToken && !m_ionAccessToken.is_empty()) {
        ionAccessToken = m_ionAccessToken;
    }
    
    PackedStringArray headers;
    headers.push_back("Authorization: Bearer " + ionAccessToken);

    Error err = m_endpointRequest->request(url, headers);
    if (err != Error::OK) {
        _emit_failure("Error starting request to endpoint: " + itos(err));
    }
}

void CesiumGDGeoJsonLoader::_on_endpoint_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body) {
    if (result != HTTPRequest::RESULT_SUCCESS || response_code != 200) {
        _emit_failure("Endpoint HTTP " + itos(response_code) + " (result=" + itos(result) + ")");
        return;
    }

    Ref<JSON> json_obj;
    json_obj.instantiate();

    String raw;
    raw.parse_utf8(reinterpret_cast<const char*>(body.ptr()), body.size());

    Error parse_err = json_obj->parse(raw);
    if (parse_err != Error::OK) {
        _emit_failure("Error parsing endpoint: " + json_obj->get_error_message());
        return;
    }

    Variant data = json_obj->get_data();
    if (data.get_type() != Variant::DICTIONARY) {
        _emit_failure("Endpoint doesn't response a JSON");
        return;
    }

    Dictionary dic_data = data;

    String asset_type = dic_data.get("type", "");
    if (asset_type != "GEOJSON") {
        _emit_failure("The asset is not a GEOJSON (type: " + asset_type + ")");
        return;
    }

    String url_gd = dic_data.get("url", "");
    String token_gd = dic_data.get("accessToken", "");
    if (url_gd.is_empty() || token_gd.is_empty()) {
        _emit_failure("Endpoint without 'url' or 'accessToken'");
        return;
    }

    _request_geojson(url_gd, token_gd);
}

// GET <url GeoJSON> using temp token
void CesiumGDGeoJsonLoader::_request_geojson(const String& url, const String& temp_token) {
    PackedStringArray headers;
    headers.push_back("Authorization: Bearer " + temp_token);

    Error err = m_geojsonRequest->request(url, headers);
    if (err != Error::OK) {
        _emit_failure("Error starting request GeoJSON: " + itos(err));
    }
}

void CesiumGDGeoJsonLoader::_on_geojson_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body) {
    if (result != HTTPRequest::RESULT_SUCCESS || response_code != 200) {
        _emit_failure("GeoJSON HTTP " + itos(response_code)
            + " (result=" + itos(result) + ")");
        return;
    }

    if (_parse_geojson(body)) {
        _emit_success();
    }
}

// Parsing GeoJSON to Array of Dictionary
bool CesiumGDGeoJsonLoader::_parse_geojson(const PackedByteArray& body) {
    Ref<JSON> json_obj;
    json_obj.instantiate();

    String raw;
    raw.parse_utf8(reinterpret_cast<const char*>(body.ptr()), body.size());

    Error parse_err = json_obj->parse(raw);
    if (parse_err != Error::OK) {
        _emit_failure("Error parsing GeoJSON: " + json_obj->get_error_message());
        return false;
    }

    Variant data = json_obj->get_data();
    if (data.get_type() != Variant::DICTIONARY) {
        _emit_failure("GeoJSON is not a valid JSON object");
        return false;
    }

    Dictionary root = data;

    if (!root.has("features") || root["features"].get_type() != Variant::ARRAY) {
        _emit_failure("GeoJSON without 'features'");
        return false;
    }

    Array raw_features = root["features"];
    Array result;

    for (int i = 0; i < raw_features.size(); i++) {
        if (raw_features[i].get_type() != Variant::DICTIONARY) continue;
        Dictionary feat = raw_features[i];
        Dictionary out;

        out["id"] = feat.get("id", Variant());

        Dictionary geom_out;
        if (feat.has("geometry") && feat["geometry"].get_type() == Variant::DICTIONARY) {
            Dictionary geom = feat["geometry"];
            geom_out["type"] = geom.get("type", String(""));
            geom_out["coordinates"] = geom.get("coordinates", Array());
        }
        
        out["geometry"] = geom_out;

        if (feat.has("properties") && feat["properties"].get_type() == Variant::DICTIONARY) {
            out["properties"] = feat["properties"];
        } else {
            out["properties"] = Dictionary();
        }

        result.push_back(out);
    }

    m_features = result;
    return true;
}

void CesiumGDGeoJsonLoader::_emit_success() {
    m_state = READY;
    emit_signal("geojson_loaded", m_features);
}

void CesiumGDGeoJsonLoader::_emit_failure(const String& msg) {
    m_state = FAILED;
    emit_signal("load_failed", msg);
}