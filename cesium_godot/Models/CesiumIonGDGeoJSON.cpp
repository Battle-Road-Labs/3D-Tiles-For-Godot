#include "CesiumIonGDGeoJSON.h"
#include "CesiumGDConfig.h"

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/io/json.h"
#include "core/object/class_db.h"
#endif


void CesiumIonGDGeoJSON::_ready() {
	m_endpointRequestNode = create_request_node();
	m_endpointRequestNode->add_request_completed_callback(
		[this](int32_t code, const PackedByteArray& body) {
			on_endpoint_request_completed(code, body);
		});

	m_geojsonRequestNode = create_request_node();
	m_geojsonRequestNode->add_request_completed_callback(
		[this](int32_t code, const PackedByteArray& body) {
			on_geojson_request_completed(code, body);
		});

	if (m_autoFetch) {
		fetch_geojson();
	}
}

void CesiumIonGDGeoJSON::set_asset_id(const String& assetId) {
	m_assetId = assetId;
}

String CesiumIonGDGeoJSON::get_asset_id() const {
	return m_assetId;
}

void CesiumIonGDGeoJSON::set_auto_fetch(bool autoFetch) {
	m_autoFetch = autoFetch;
}

bool CesiumIonGDGeoJSON::get_auto_fetch() const {
	return m_autoFetch;
}

String CesiumIonGDGeoJSON::get_raw_geojson() const {
	return m_rawGeoJSON;
}

Dictionary CesiumIonGDGeoJSON::get_parsed_geojson() const {
	return m_parsedGeoJSON;
}

void CesiumIonGDGeoJSON::fetch_geojson() {
	if (m_state == FetchState::FetchingEndpoint || m_state == FetchState::FetchingGeoJSON) {
		WARN_PRINT("CesiumIonGeoJSONNode: fetch already in progress, ignoring call");
		return;
	}

	if (m_assetId.is_empty()) {
		const String error = "CesiumIonGeoJSONNode: asset_id is empty";
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		return;
	}

	CesiumGDConfig* config = CesiumGDConfig::get_singleton(this);
	if (!config) {
		const String error = "CesiumIonGeoJSONNode: CesiumGDConfig singleton not found in scene";
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		return;
	}

	const String token = config->get_access_token();
	if (token.is_empty()) {
		const String error = "CesiumIonGeoJSONNode: Cesium Ion access token is empty";
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		return;
	}

	const String endpointUrl = String(CesiumGDConfig::DEFAULT_ION_API_URL.data()) + "v1/assets/" + m_assetId + "/endpoint";

	PackedStringArray headers;
	headers.append("Authorization: Bearer " + token);

	m_state = FetchState::FetchingEndpoint;

	const Error err = m_endpointRequestNode->request(endpointUrl, headers);
	if (err != OK) {
		const String error = "CesiumIonGeoJSONNode: failed to initiate endpoint request, error code: " + itos(err);
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
	}
}

CesiumHTTPRequestNode* CesiumIonGDGeoJSON::create_request_node() {
	CesiumHTTPRequestNode* node = memnew(CesiumHTTPRequestNode);
	add_child(node);
	node->connect("request_completed", Callable(node, "on_request_completed"));
	return node;
}

void CesiumIonGDGeoJSON::on_endpoint_request_completed(int32_t responseCode, const PackedByteArray& body) {
	if (responseCode != 200) {
		const String error = "CesiumIonGeoJSONNode: endpoint request failed, HTTP " + itos(responseCode);
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
		return;
	}

	const String responseStr = body.get_string_from_utf8();

	Ref<JSON> json;
	json.instantiate();
	const Error parseErr = json->parse(responseStr);
	if (parseErr != OK) {
		const String error = "CesiumIonGeoJSONNode: failed to parse endpoint JSON response";
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
		return;
	}

	const Dictionary data = json->get_data();
	const String url = data.get("url", String(""));
	const String accessToken = data.get("accessToken", String(""));

	if (url.is_empty()) {
		const String error = "CesiumIonGeoJSONNode: endpoint response is missing the 'url' field";
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
		return;
	}

	m_state = FetchState::FetchingGeoJSON;

	PackedStringArray headers;
	if (!accessToken.is_empty()) {
		headers.append("Authorization: Bearer " + accessToken);
	}

	const Error err = m_geojsonRequestNode->request(url, headers);
	if (err != OK) {
		const String error = "CesiumIonGeoJSONNode: failed to initiate GeoJSON download, error code: " + itos(err);
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
	}
}

void CesiumIonGDGeoJSON::on_geojson_request_completed(int32_t responseCode, const PackedByteArray& body) {
	if (responseCode != 200) {
		const String error = "CesiumIonGeoJSONNode: GeoJSON download failed, HTTP " + itos(responseCode);
		ERR_PRINT(error);
		emit_signal("geojson_fetch_failed", error);
		m_state = FetchState::Error;
		return;
	}

	m_rawGeoJSON = body.get_string_from_utf8();

	Ref<JSON> json;
	json.instantiate();
	const Error parseErr = json->parse(m_rawGeoJSON);
	if (parseErr == OK) {
		const Variant parsed = json->get_data();
		if (parsed.get_type() == Variant::DICTIONARY) {
			m_parsedGeoJSON = parsed;
		} else {
			WARN_PRINT("CesiumIonGeoJSONNode: GeoJSON root is not a JSON object – parsed dictionary will be empty");
			m_parsedGeoJSON = Dictionary();
		}
	} else {
		WARN_PRINT("CesiumIonGeoJSONNode: could not parse GeoJSON response – raw string is still available");
		m_parsedGeoJSON = Dictionary();
	}

	m_state = FetchState::Done;
	emit_signal("geojson_loaded", m_rawGeoJSON, m_parsedGeoJSON);
}

void CesiumIonGDGeoJSON::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_asset_id", "asset_id"), &CesiumIonGDGeoJSON::set_asset_id);
	ClassDB::bind_method(D_METHOD("get_asset_id"), &CesiumIonGDGeoJSON::get_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "asset_id"), "set_asset_id", "get_asset_id");

	ClassDB::bind_method(D_METHOD("set_auto_fetch", "auto_fetch"), &CesiumIonGDGeoJSON::set_auto_fetch);
	ClassDB::bind_method(D_METHOD("get_auto_fetch"), &CesiumIonGDGeoJSON::get_auto_fetch);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_fetch"), "set_auto_fetch", "get_auto_fetch");

	ClassDB::bind_method(D_METHOD("fetch_geojson"), &CesiumIonGDGeoJSON::fetch_geojson);
	ClassDB::bind_method(D_METHOD("get_raw_geojson"), &CesiumIonGDGeoJSON::get_raw_geojson);
	ClassDB::bind_method(D_METHOD("get_parsed_geojson"), &CesiumIonGDGeoJSON::get_parsed_geojson);

	ADD_SIGNAL(MethodInfo("geojson_loaded", PropertyInfo(Variant::STRING, "raw"), PropertyInfo(Variant::DICTIONARY, "parsed")));
	ADD_SIGNAL(MethodInfo("geojson_fetch_failed", PropertyInfo(Variant::STRING, "error")));
}
