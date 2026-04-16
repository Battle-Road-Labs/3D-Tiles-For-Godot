#ifndef WEB_FETCH_CLIENT_H
#define WEB_FETCH_CLIENT_H

#ifndef __EMSCRIPTEN__
#error "WebFetchClient.h is only intended for Emscripten/Web builds"
#endif

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/http_client.hpp"
#include "godot_cpp/classes/marshalls.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/string.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/error/error_macros.h"
#include "core/io/http_client.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "core/variant/variant.h"
#endif

using WebFetchCallback_t = std::function<void(int32_t, const PackedByteArray&)>;
using WebFetchHeader_t = std::pair<std::string, std::string>;

namespace cesium_web_fetch_detail {

struct PendingRequest {
	WebFetchCallback_t callback;
	std::string url;
};

std::mutex& registry_mutex();
std::unordered_map<int32_t, std::unique_ptr<PendingRequest>>& registry();
std::atomic<int32_t>& next_id();

// Implemented in WebFetchClient.cpp. Kicks off the browser fetch via EM_JS.
// Headers are consumed synchronously during this call, so header_kv memory may be
// released on return. Body bytes (if any) are copied into a JS-owned buffer.
void start_fetch(int32_t request_id,
				 const char* method,
				 const char* url,
				 const char* const* headers_kv,
				 const uint8_t* body,
				 int32_t body_len);

} // namespace cesium_web_fetch_detail

/// @brief HTTP client that routes through browser fetch() via EM_JS.
/// Avoids Godot's HTTPClientWeb spin-polling: fetch is natively async, there is
/// no main-thread proxy storm, and no "polled multiple times in one frame" warning.
/// N_MAX_CONCURRENT is accepted for API parity with GodotHttpClient but unused —
/// the browser already limits per-origin concurrency.
template<uint32_t N_MAX_CONCURRENT>
class WebFetchClient {
	static_assert(N_MAX_CONCURRENT <= 100);

public:
	WebFetchClient() = default;
	~WebFetchClient() = default;

	void init_client(size_t /*maxThreads*/) {
		// No thread pool needed; fetch is async at the browser level.
		std::string systemInfo = OS::get_singleton()->get_name().utf8().get_data();
		auto architecture = OS::get_singleton()->get_processor_name().utf8().get_data();
		std::stringstream stream;
		stream << "Godot3DTiles/1.0 (" << systemInfo << "; " << architecture << ")";
		add_default_header({"User-Agent", stream.str()});
	}

	void add_default_header(const WebFetchHeader_t& header) {
		m_defaultHeaders.emplace_back(header);
	}

	void send_get(const char* url, const WebFetchCallback_t& callback, const std::vector<WebFetchHeader_t>& headers) {
		send_request(url, HTTPClient::METHOD_GET, callback, headers);
	}

	void send_request(const char* url, HTTPClient::Method method,
					  const WebFetchCallback_t& callback,
					  const std::vector<WebFetchHeader_t>& headers) {
		std::string urlStr(url);

		// data: URIs contain inline base64 (Cesium credit logos).
		// Browser fetch() accepts them, but decoding locally avoids a round-trip.
		if (urlStr.rfind("data:", 0) == 0) {
			size_t base64Pos = urlStr.find(";base64,");
			if (base64Pos != std::string::npos) {
				String gdBase64 = String(urlStr.substr(base64Pos + 8).c_str());
				PackedByteArray buf = Marshalls::get_singleton()->base64_to_raw(gdBase64);
				callback(HTTPClient::RESPONSE_OK, buf);
				return;
			}
			ERR_PRINT(String("Invalid data URI format: ") + url);
			callback(HTTPClient::RESPONSE_BAD_REQUEST, PackedByteArray());
			return;
		}

		const char* methodStr = method_to_string(method);

		std::vector<WebFetchHeader_t> combined;
		combined.reserve(m_defaultHeaders.size() + headers.size());
		combined.insert(combined.end(), m_defaultHeaders.begin(), m_defaultHeaders.end());
		combined.insert(combined.end(), headers.begin(), headers.end());

		// Flat null-terminated array of [key, value, key, value, ..., null] pointers.
		// JS walks this synchronously inside start_fetch — safe to let it go out of scope after.
		std::vector<const char*> kvPtrs;
		kvPtrs.reserve(combined.size() * 2 + 1);
		for (const auto& h : combined) {
			kvPtrs.push_back(h.first.c_str());
			kvPtrs.push_back(h.second.c_str());
		}
		kvPtrs.push_back(nullptr);

		auto ctx = std::make_unique<cesium_web_fetch_detail::PendingRequest>();
		ctx->callback = callback;
		ctx->url = urlStr;

		int32_t req_id = cesium_web_fetch_detail::next_id().fetch_add(1);
		{
			std::lock_guard<std::mutex> lock(cesium_web_fetch_detail::registry_mutex());
			cesium_web_fetch_detail::registry()[req_id] = std::move(ctx);
		}

		cesium_web_fetch_detail::start_fetch(
			req_id, methodStr, urlStr.c_str(),
			kvPtrs.data(), nullptr, 0);
	}

private:
	static const char* method_to_string(HTTPClient::Method method) {
		switch (method) {
			case HTTPClient::METHOD_GET:     return "GET";
			case HTTPClient::METHOD_POST:    return "POST";
			case HTTPClient::METHOD_PUT:     return "PUT";
			case HTTPClient::METHOD_DELETE:  return "DELETE";
			case HTTPClient::METHOD_HEAD:    return "HEAD";
			case HTTPClient::METHOD_OPTIONS: return "OPTIONS";
			case HTTPClient::METHOD_PATCH:   return "PATCH";
			default:                         return "GET";
		}
	}

	std::vector<WebFetchHeader_t> m_defaultHeaders;
};

#endif // WEB_FETCH_CLIENT_H
