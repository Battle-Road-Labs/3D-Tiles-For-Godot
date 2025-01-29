#include "CesiumHTTPRequestNode.h"
#include "LocalAssetResponse.h"
#include "LocalAssetRequest.h"
#include <CesiumAsync/AsyncSystem.h>

CesiumHTTPRequestNode::CesiumHTTPRequestNode()
{
	this->set_use_threads(true);
	this->set_accept_gzip(true);
}

void CesiumHTTPRequestNode::add_request_completed_callback(std::function<void(int32_t responseCode, const PackedByteArray& body)> func)
{
	this->m_requestCompletedCallback = func;
}

void CesiumHTTPRequestNode::on_request_completed(int result, int responseCode, const PackedStringArray& headers, const PackedByteArray& body)
{

	if (result != HTTPRequest::RESULT_SUCCESS) {
		ERR_PRINT(String("Cesium Request failed to connect to the server, data source service might be unavailable, error status: ") + itos(result));
	}
	if (this->m_requestCompletedCallback) {
		this->m_requestCompletedCallback(responseCode, body);
	}
	this->m_grabbed = false;
}

void CesiumHTTPRequestNode::set_grabbed_status(bool grabbed)
{
	this->m_grabbed = grabbed;
}

bool CesiumHTTPRequestNode::is_grabbed() const
{
	return this->m_grabbed;
}

void CesiumHTTPRequestNode::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("on_request_completed", "result", "responseCode", "headers", "body"), &CesiumHTTPRequestNode::on_request_completed);
}
