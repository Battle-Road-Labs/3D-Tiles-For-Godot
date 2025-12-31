#ifndef GODOT_HTTP_CLIENT_H
#define GODOT_HTTP_CLIENT_H

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/templates/vector.hpp"
#include "godot_cpp/classes/http_client.hpp"
#include "godot_cpp/classes/marshalls.hpp"
#include "godot_cpp/classes/tls_options.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/templates/vector.h"
#include "core/io/marshalls.h"
#include "core/io/http_client.h"
#endif

#include "BRThreadPool.h"
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using GodotResponseCallback_t = std::function<void(int32_t, const PackedByteArray&)>;
using GodotHeader_t = std::pair<std::string, std::string>;

/// @brief HTTP client using Godot's native HTTPClient instead of libcurl.
/// This provides better cross-platform compatibility, especially on macOS
/// where curl has DNS resolution and SSL/SNI issues.
template<uint32_t N_MAX_CONCURRENT>
class GodotHttpClient {
	static_assert(N_MAX_CONCURRENT <= 100);

public:
	GodotHttpClient() = default;
	~GodotHttpClient() = default;

	void init_client(size_t maxThreads) {
		m_threadPool.init(maxThreads);
		std::string systemInfo = OS::get_singleton()->get_name().utf8().get_data();
		auto architecture = OS::get_singleton()->get_processor_name().utf8().get_data();
		std::stringstream stream;
		stream << "Godot3DTiles/1.0 (" << systemInfo << "; " << architecture << ")";
		add_default_header({"User-Agent", stream.str()});
	}

	void send_get(const char* url, const GodotResponseCallback_t& callback, const std::vector<GodotHeader_t>& headers) {
		send_request(url, HTTPClient::METHOD_GET, callback, headers);
	}

	void send_get_same_thread(const char* url, const GodotResponseCallback_t& callback, const std::vector<GodotHeader_t>& headers) {
		send_request_same_thread(url, HTTPClient::METHOD_GET, callback, headers);
	}

	void send_request(const char* url, HTTPClient::Method method, const GodotResponseCallback_t& callback, const std::vector<GodotHeader_t>& headers) {
		std::string urlCopy = url;
		std::vector<GodotHeader_t> allHeaders = m_defaultHeaders;
		allHeaders.insert(allHeaders.end(), headers.begin(), headers.end());

		m_threadPool.enqueue([this, urlCopy, method, callback, allHeaders] {
			int32_t responseCode = 0;
			PackedByteArray body = perform_request(urlCopy.c_str(), method, &responseCode, allHeaders);
			callback(responseCode, body);
		});
	}

	void send_request_same_thread(const char* url, HTTPClient::Method method, const GodotResponseCallback_t& callback, const std::vector<GodotHeader_t>& headers) {
		std::vector<GodotHeader_t> allHeaders = m_defaultHeaders;
		allHeaders.insert(allHeaders.end(), headers.begin(), headers.end());

		int32_t responseCode = 0;
		PackedByteArray body = perform_request(url, method, &responseCode, allHeaders);
		callback(responseCode, body);
	}

	void add_default_header(const GodotHeader_t& header) {
		m_defaultHeaders.emplace_back(header);
	}

private:
	struct UrlParts {
		std::string scheme;
		std::string host;
		int port;
		std::string path;
		bool use_tls;
		bool valid;
	};

	UrlParts parse_url(const std::string& url) {
		UrlParts parts{};
		parts.valid = false;
		parts.port = -1;

		size_t schemeEnd = url.find("://");
		if (schemeEnd == std::string::npos) {
			return parts;
		}

		parts.scheme = url.substr(0, schemeEnd);
		parts.use_tls = (parts.scheme == "https");

		size_t hostStart = schemeEnd + 3;
		size_t pathStart = url.find('/', hostStart);

		std::string hostPart;
		if (pathStart == std::string::npos) {
			hostPart = url.substr(hostStart);
			parts.path = "/";
		} else {
			hostPart = url.substr(hostStart, pathStart - hostStart);
			parts.path = url.substr(pathStart);
		}

		size_t portPos = hostPart.find(':');
		if (portPos != std::string::npos) {
			parts.host = hostPart.substr(0, portPos);
			try {
				parts.port = std::stoi(hostPart.substr(portPos + 1));
			} catch (...) {
				return parts;
			}
		} else {
			parts.host = hostPart;
			parts.port = parts.use_tls ? 443 : 80;
		}

		parts.valid = !parts.host.empty();
		return parts;
	}

	PackedByteArray perform_request(const char* url, HTTPClient::Method method, int32_t* outStatus, const std::vector<GodotHeader_t>& headers) {
		PackedByteArray buffer;
		*outStatus = 0;

		std::string urlStr(url);

		// Handle data: URIs (base64-encoded inline data, used by Cesium credit logos)
		if (urlStr.rfind("data:", 0) == 0) {
			size_t base64Pos = urlStr.find(";base64,");
			if (base64Pos != std::string::npos) {
				std::string base64Data = urlStr.substr(base64Pos + 8);
				String godotBase64 = String(base64Data.c_str());
				buffer = Marshalls::get_singleton()->base64_to_raw(godotBase64);
				*outStatus = 200;
				return buffer;
			}
			ERR_PRINT(String("Invalid data URI format: ") + url);
			*outStatus = 400;
			return PackedByteArray();
		}

		UrlParts parts = parse_url(urlStr);
		if (!parts.valid) {
			ERR_PRINT(String("Invalid URL format: ") + url);
			*outStatus = 400;
			return PackedByteArray();
		}

		// Create HTTPClient instance for this request
		Ref<HTTPClient> http;
		http.instantiate();

		// Configure TLS for HTTPS
		Ref<TLSOptions> tls_options;
		if (parts.use_tls) {
			tls_options = TLSOptions::client();
		}

		// Connect to host
		Error connectErr = http->connect_to_host(String(parts.host.c_str()), parts.port, tls_options);
		if (connectErr != Error::OK) {
			ERR_PRINT(String("Failed to initiate connection to: ") + parts.host.c_str() + " error: " + itos(static_cast<int>(connectErr)));
			*outStatus = 0;
			return PackedByteArray();
		}

		// Poll until connected (max ~10 seconds)
		constexpr int MAX_CONNECT_POLLS = 1000;
		constexpr int POLL_DELAY_MS = 10;
		int polls = 0;

		while (http->get_status() == HTTPClient::STATUS_CONNECTING ||
		       http->get_status() == HTTPClient::STATUS_RESOLVING) {
			http->poll();
			std::this_thread::sleep_for(std::chrono::milliseconds(POLL_DELAY_MS));
			polls++;
			if (polls > MAX_CONNECT_POLLS) {
				ERR_PRINT(String("Connection timeout for: ") + url);
				*outStatus = 0;
				return PackedByteArray();
			}
		}

		if (http->get_status() != HTTPClient::STATUS_CONNECTED) {
			ERR_PRINT(String("Failed to connect to: ") + parts.host.c_str() + " status: " + itos(static_cast<int>(http->get_status())));
			*outStatus = 0;
			return PackedByteArray();
		}

		// Build headers array
		PackedStringArray godotHeaders;
		for (const auto& header : headers) {
			String headerStr = String(header.first.c_str()) + ": " + String(header.second.c_str());
			godotHeaders.push_back(headerStr);
		}

		// Send request
		Error requestErr = http->request(method, String(parts.path.c_str()), godotHeaders);
		if (requestErr != Error::OK) {
			ERR_PRINT(String("Failed to send request to: ") + url + " error: " + itos(static_cast<int>(requestErr)));
			*outStatus = 0;
			return PackedByteArray();
		}

		// Poll until request completes (max ~60 seconds for large files)
		constexpr int MAX_REQUEST_POLLS = 6000;
		polls = 0;

		while (http->get_status() == HTTPClient::STATUS_REQUESTING) {
			http->poll();
			std::this_thread::sleep_for(std::chrono::milliseconds(POLL_DELAY_MS));
			polls++;
			if (polls > MAX_REQUEST_POLLS) {
				ERR_PRINT(String("Request timeout for: ") + url);
				*outStatus = 0;
				return PackedByteArray();
			}
		}

		if (!http->has_response()) {
			ERR_PRINT(String("No response from: ") + url + " status: " + itos(static_cast<int>(http->get_status())));
			*outStatus = 0;
			return PackedByteArray();
		}

		*outStatus = http->get_response_code();

		// Read response body
		while (http->get_status() == HTTPClient::STATUS_BODY) {
			http->poll();
			PackedByteArray chunk = http->read_response_body_chunk();
			if (chunk.size() > 0) {
				buffer.append_array(chunk);
			}
			if (chunk.size() == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		http->close();
		return buffer;
	}

	BRThreadPool m_threadPool;
	std::vector<GodotHeader_t> m_defaultHeaders;
};

#endif // GODOT_HTTP_CLIENT_H
