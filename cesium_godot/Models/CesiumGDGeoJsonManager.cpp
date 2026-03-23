#include "CesiumGDGeoJsonManager.h"
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

CesiumGDGeoJsonManager::CesiumGDGeoJsonManager() {}

void CesiumGDGeoJsonManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_assets_search", "v"), &CesiumGDGeoJsonManager::set_assets_search);
    ClassDB::bind_method(D_METHOD("get_assets_search"), &CesiumGDGeoJsonManager::get_assets_search);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "assets_search"), "set_assets_search", "get_assets_search");

    ClassDB::bind_method(D_METHOD("set_fetsch_geojsons_at_startup", "v"), &CesiumGDGeoJsonManager::set_fetsch_geojsons_at_startup);
    ClassDB::bind_method(D_METHOD("get_fetsch_geojsons_at_startup"), &CesiumGDGeoJsonManager::get_fetsch_geojsons_at_startup);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fetch_geojsons_at_startup"), "set_fetsch_geojsons_at_startup", "get_fetsch_geojsons_at_startup");

    ClassDB::bind_method(D_METHOD("set_auto_load_geojsons", "v"), &CesiumGDGeoJsonManager::set_auto_load_geojsons);
    ClassDB::bind_method(D_METHOD("get_auto_load_geojsons"), &CesiumGDGeoJsonManager::get_auto_load_geojsons);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_load_geojsons", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_auto_load_geojsons", "get_auto_load_geojsons");

    ClassDB::bind_method(D_METHOD("set_assetData", "v"), &CesiumGDGeoJsonManager::set_assetData);
    ClassDB::bind_method(D_METHOD("get_assetData"), &CesiumGDGeoJsonManager::get_assetData);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "assetData"), "set_assetData", "get_assetData");

    ClassDB::bind_method(D_METHOD("set_geojsonLoaders", "v"), &CesiumGDGeoJsonManager::set_geojsonLoaders);
    ClassDB::bind_method(D_METHOD("get_geojsonLoaders"), &CesiumGDGeoJsonManager::get_geojsonLoaders);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "geojsonLoaders"), "set_geojsonLoaders", "get_geojsonLoaders");

    ClassDB::bind_method(D_METHOD("fetch_assets"), &CesiumGDGeoJsonManager::fetch_assets);
    ClassDB::bind_method(D_METHOD("load_all"), &CesiumGDGeoJsonManager::load_all);
    ClassDB::bind_method(D_METHOD("get_asset_infos"), &CesiumGDGeoJsonManager::get_asset_infos);
    ClassDB::bind_method(D_METHOD("get_loaders"), &CesiumGDGeoJsonManager::get_loaders);
    ClassDB::bind_method(D_METHOD("get_loader_by_id", "asset_id"), &CesiumGDGeoJsonManager::get_loader_by_id);

    ClassDB::bind_method(D_METHOD("_on_assets_completed", "result", "response_code", "headers", "body"), &CesiumGDGeoJsonManager::_on_assets_completed);

    ADD_SIGNAL(MethodInfo("assets_fetched", PropertyInfo(Variant::ARRAY, "asset_infos")));
    ADD_SIGNAL(MethodInfo("loaders_ready", PropertyInfo(Variant::ARRAY, "loaders")));
    ADD_SIGNAL(MethodInfo("fetch_failed", PropertyInfo(Variant::STRING, "error")));
}

void CesiumGDGeoJsonManager::_ready() {
    m_assetsRequest = memnew(HTTPRequest);
    m_assetsRequest->set_name("AssetsReq");
    m_assetsRequest->set_use_threads(true);
    m_assetsRequest->set_timeout(20.0);
    add_child(m_assetsRequest);

    m_assetsRequest->connect("request_completed",
        Callable(this, "_on_assets_completed"));

    if (m_fetchGeojsonsAtStartup) {
        fetch_assets();
    }
}

// GET https://api.cesium.com/v1/assets
void CesiumGDGeoJsonManager::fetch_assets() {
    String token = CesiumGDConfig::get_singleton(this)->get_access_token();
    if (token.is_empty()) {
        _emit_failure("There is no token in CesiumGDConfig");
        return;
    }

    String querty_str = "";
    if (!assets_search.is_empty()) {
        querty_str += "search=" + assets_search;
    }

    String url = "https://api.cesium.com/v1/assets";
    if (!querty_str.is_empty()) {
        url += "?" + querty_str;
    }
  
    PackedStringArray headers;
    headers.push_back("Authorization: Bearer " + token);

    Error err = m_assetsRequest->request(url, headers);
    if (err != Error::OK) {
        _emit_failure("Error starting request the assets: " + itos(err));
    }
}

void CesiumGDGeoJsonManager::_on_assets_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body) {
    if (result != HTTPRequest::RESULT_SUCCESS || response_code != 200) {
        _emit_failure("Assets HTTP " + itos(response_code) + " (result=" + itos(result) + ")");
        return;
    }

    Ref<JSON> json_obj;
    json_obj.instantiate();

    String raw;
    raw.parse_utf8(reinterpret_cast<const char*>(body.ptr()), body.size());

    if (json_obj->parse(raw) != Error::OK) {
        _emit_failure("Error parsing asset response: " + json_obj->get_error_message());
        return;
    }

    Dictionary data = json_obj->get_data();
    if (!data.has("items") || data["items"].get_type() != Variant::ARRAY) {
        _emit_failure("Answer without 'items'");
        return;
    }

    assetData.clear();
    Array items = data["items"];
    for (int i = 0; i < items.size(); i++) {
        if (items[i].get_type() != Variant::DICTIONARY) continue;
        Dictionary item = items[i];

        String type = item.get("type", "");
        if (type != "GEOJSON") continue;

        Dictionary asset_data;
        asset_data["id"] = item.get("id", 0);
        asset_data["name"] = item.get("name", "");
        asset_data["type"] = item.get("type", "");
        asset_data["description"] = item.get("description", "");
        asset_data["bytes"] = item.get("bytes", 0);
        asset_data["attribution"] = item.get("attribution", "");
        asset_data["dateAdded"] = item.get("dateAdded", "");
        asset_data["status"] = item.get("status", "");
        asset_data["percentComplete"] = item.get("percentComplete", 0.0);
        asset_data["archivable"] = item.get("archivable", false);
        asset_data["exportable"] = item.get("exportable", false);
        assetData.push_back(asset_data);
    }

    emit_signal("assets_fetched", assetData);

    _create_loaders();
}

void CesiumGDGeoJsonManager::_create_loaders() {
    for (int idx = 0; idx < geojsonLoaders.size(); idx++) {
        Object* obj = Object::cast_to<Object>(geojsonLoaders[idx]);
        if (obj) {
            Node* node = Object::cast_to<Node>(obj);
            if (node) node->queue_free();
        }
    }

    geojsonLoaders.clear();

    for (int i = 0; i < assetData.size(); i++) {
        Dictionary asset_d = assetData[i];
        int64_t asset_id = asset_d["id"];
        String name = asset_d["name"];

        CesiumGDGeoJsonLoader* loader = memnew(CesiumGDGeoJsonLoader);
        loader->set_name("GeoJson_" + godot::String::num_int64(asset_id, 10));
        loader->set_ion_asset_id(asset_id);
        loader->set_auto_load(m_autoLoadGeojsons);
        add_child(loader);

        geojsonLoaders.push_back(loader);
    }

    emit_signal("loaders_ready", geojsonLoaders);
}

CesiumGDGeoJsonLoader* CesiumGDGeoJsonManager::get_loader_by_id(int64_t asset_id) const {
    for (int i = 0; i < geojsonLoaders.size(); i++) {
        CesiumGDGeoJsonLoader* loader =
            Object::cast_to<CesiumGDGeoJsonLoader>(geojsonLoaders[i]);
        if (loader && loader->get_ion_asset_id() == asset_id) {
            return loader;
        }
    }
    return nullptr;
}

void CesiumGDGeoJsonManager::load_all() {
    for (int i = 0; i < geojsonLoaders.size(); i++) {
        CesiumGDGeoJsonLoader* loader =
            Object::cast_to<CesiumGDGeoJsonLoader>(geojsonLoaders[i]);
        if (loader) loader->load();
    }
}

void CesiumGDGeoJsonManager::_emit_failure(const String& msg) {
    emit_signal("fetch_failed", msg);
}

void CesiumGDGeoJsonManager::_get_property_list(List<PropertyInfo>* p_list) const {
    uint32_t usage = m_fetchGeojsonsAtStartup ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NO_EDITOR;

    p_list->push_back(PropertyInfo(
        Variant::BOOL,
        "auto_load_geojsons",
        PROPERTY_HINT_NONE,
        "",
        usage
    ));
}

bool CesiumGDGeoJsonManager::_get(const StringName& p_name, Variant& r_property)  const {
    if (p_name == StringName("auto_load_geojsons")) {
        r_property = m_autoLoadGeojsons;
        return true;
    }
    return false;
}

bool CesiumGDGeoJsonManager::_set(const StringName& p_name, const Variant& p_property) {
    if (p_name == StringName("auto_load_geojsons")) {
        m_autoLoadGeojsons = p_property;
        return true;
    }
    return false;
}