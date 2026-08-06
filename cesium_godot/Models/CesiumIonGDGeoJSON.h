#ifndef CESIUM_ION_GEOJSON_NODE_H
#define CESIUM_ION_GEOJSON_NODE_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/main/node.h"
#endif

#include "CesiumHTTPRequestNode.h"


class CesiumIonGDGeoJSON : public Node {
	GDCLASS(CesiumIonGDGeoJSON, Node)

public:
	CesiumIonGDGeoJSON() = default;

	void set_asset_id(const String& assetId);
	String get_asset_id() const;

	void set_auto_fetch(bool autoFetch);
	bool get_auto_fetch() const;

	void fetch_geojson();

	String get_raw_geojson() const;

	Dictionary get_parsed_geojson() const;

	void _ready() override;

private:
	enum class FetchState { Idle, FetchingEndpoint, FetchingGeoJSON, Done, Error };

	String m_assetId;
	bool m_autoFetch = false;
	String m_rawGeoJSON;
	Dictionary m_parsedGeoJSON;
	FetchState m_state = FetchState::Idle;

	CesiumHTTPRequestNode* m_endpointRequestNode = nullptr;
	CesiumHTTPRequestNode* m_geojsonRequestNode = nullptr;

	void on_endpoint_request_completed(int32_t responseCode, const PackedByteArray& body);
	void on_geojson_request_completed(int32_t responseCode, const PackedByteArray& body);

	CesiumHTTPRequestNode* create_request_node();

protected:
	static void _bind_methods();
};

#endif // CESIUM_ION_GEOJSON_NODE_H
