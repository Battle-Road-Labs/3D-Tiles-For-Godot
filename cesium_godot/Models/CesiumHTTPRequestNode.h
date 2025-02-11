#ifndef CESIUM_HTTP_REQUEST_NODE_H
#define CESIUM_HTTP_REQUEST_NODE_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/http_request.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include <scene/main/http_request.h>
#endif

#include <CesiumAsync/Future.h>
#include <CesiumAsync/Promise.h>
#include <CesiumAsync/IAssetRequest.h>

class CesiumHTTPRequestNode : public HTTPRequest
{
	GDCLASS(CesiumHTTPRequestNode, HTTPRequest)

public:

	CesiumHTTPRequestNode();

	void add_request_completed_callback(std::function<void(int32_t responseCode, const PackedByteArray& body)> func);

	void on_request_completed(int result, int responseCode, const PackedStringArray& headers, const PackedByteArray& body);

	void set_grabbed_status(bool grabbed);

	bool is_grabbed() const;

private:
	std::function<void(int32_t responseCode, const PackedByteArray& body)> m_requestCompletedCallback;

	/**
	 * @brief Will be set to true if the request node is being picked for a new request, will be set to false once the request is completed
	 */
	bool m_grabbed = false;

protected:
	static void _bind_methods();
};

#endif
