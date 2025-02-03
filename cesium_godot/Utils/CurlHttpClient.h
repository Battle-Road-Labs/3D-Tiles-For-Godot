#ifndef CURL_HTTP_CLIENT
#define CURL_HTTP_CLIENT

#if defined(CESIUM_GD_EXT)

#ifdef _WIN32
//Link to windows libraries
#pragma comment(lib, "Ws2_32.Lib")
#pragma comment(lib, "Wldap32.Lib")
#pragma comment(lib, "Crypt32.Lib")
#endif

#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include <godot_cpp/templates/vector.hpp>
#include "godot_cpp/classes/http_client.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/templates/vector.h"
#endif

#include "BRThreadPool.h"
#include <curl/curl.h>
#include <vector>
#include <string>

using HighLevelResponseCallback_t = std::function<void(int32_t, const Vector<uint8_t>&)>;
using CesiumHeader_t = std::pair<std::string, std::string>;

struct RequestHandle_t {
	CURL *curlHandle;
	bool available;

	RequestHandle_t() : curlHandle(nullptr), available(true) {}

	RequestHandle_t(const RequestHandle_t &other) {
		this->curlHandle = other.curlHandle;
		this->available = other.available;
	}

	RequestHandle_t &operator=(const RequestHandle_t &other) {
		if (this != &other) {
			this->curlHandle = other.curlHandle;
			this->available = other.available;
		}
		return *this;
	}

	void easy_init() {
		this->curlHandle = curl_easy_init();
		this->available = true;
		curl_easy_setopt(this->curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(this->curlHandle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(this->curlHandle, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(this->curlHandle, CURLOPT_VERBOSE, 0);
	}

	void easy_cleanup() {
		curl_easy_cleanup(this->curlHandle);
	}

	void configure_http_method(HTTPClient::Method method) {
		switch (method) {
			case HTTPClient::METHOD_POST:
				curl_easy_setopt(this->curlHandle, CURLOPT_POST, 1);
				break;
			case HTTPClient::METHOD_PUT:
				curl_easy_setopt(this->curlHandle, CURLOPT_PUT, 1);
				break;
			default:
				break;
		}
	}

};

struct MemoryStruct {
	char *memory;
	size_t size;
};

/// @brief Wrapper around libcurl, but without worrying about polling constantly
template<uint32_t N_MAX_HANDLES>
class CurlHttpClient {
	static_assert(N_MAX_HANDLES <= 80); //Just to handle around 80 handles at once, multiple transfers will happen on the same handle

public:
	CurlHttpClient() {
		if (s_activeInstances == 0) {
			curl_global_init(CURL_GLOBAL_ALL);
		}
		s_activeInstances++;
		this->m_activeHandles.reserve(N_MAX_HANDLES);
		//Init all handles
		for (int32_t i = 0; i < N_MAX_HANDLES; i++) {
			RequestHandle_t handle{};
			handle.easy_init();
			this->m_activeHandles.push_back(handle);
		}
	}

	~CurlHttpClient() {
		for (int32_t i = 0; i < this->m_activeHandles.size(); i++) {
			this->m_activeHandles[i].easy_cleanup();
		}

		s_activeInstances--;
		if (s_activeInstances == 0) {
			curl_global_cleanup();
		}
	}

	void init_client(size_t maxThreads) {
		this->m_threadPool.init(maxThreads);		
	}

	void send_get(const char* url, const HighLevelResponseCallback_t& callback, const std::vector<CesiumHeader_t>& headers) {
		// Just send a request here
		this->send_request(url, HTTPClient::METHOD_GET, callback, headers);
	}

	void send_get_same_thread(const char *url, const HighLevelResponseCallback_t &callback, const std::vector<CesiumHeader_t> &headers) {
		// Just send a request here
		this->send_request_same_thread(url, HTTPClient::METHOD_GET, callback, headers);
	}

	void send_request(const char* url, HTTPClient::Method method, const HighLevelResponseCallback_t& callback, const std::vector<CesiumHeader_t>& headers) {
		//Get any active handle
		int32_t handleIdx = this->get_available_handle_index();
		this->m_activeHandles[handleIdx].available = false;
		std::string urlCopy = url;
		this->m_threadPool.enqueue([&, urlCopy, headers, callback, handleIdx] {
			RequestHandle_t& handle = this->m_activeHandles[handleIdx];
			handle.configure_http_method(method);
			long responseCode;
			Vector<uint8_t> packedData = pull_url(urlCopy.c_str(), handle.curlHandle, &responseCode, headers);
			//And call the callback methods here
			callback(responseCode, packedData);
			handle.available = true;
		});
	}


	void send_request_same_thread(const char *url, HTTPClient::Method method, const HighLevelResponseCallback_t &callback, const std::vector<CesiumHeader_t> &headers) {
		//Get any active handle
		int32_t handleIdx = this->get_available_handle_index();
		this->m_activeHandles[handleIdx].available = false;
		RequestHandle_t &handle = this->m_activeHandles[handleIdx];
		handle.configure_http_method(method);
		long responseCode;
		Vector<uint8_t> packedData = pull_url(url, handle.curlHandle, &responseCode, headers);
		//And call the callback methods here
		callback(responseCode, packedData);
		handle.available = true;
	}


private:
	static inline uint16_t s_activeInstances = 0;

	Vector<uint8_t> pull_url(const char *url, CURL *handle, long*outStatus , const std::vector<CesiumHeader_t> &headers) {
		Vector<uint8_t> buffer;
		//Options stuff
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &CurlHttpClient::write_callback);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");


		curl_slist* curlHeaders = nullptr;
		for (const CesiumHeader_t& h : headers)
		{
			std::string strHeader = h.first + ": ";
			strHeader += h.second;
			curlHeaders = curl_slist_append(curlHeaders, strHeader.c_str());
		}

		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, curlHeaders);

		//Perform the request		
		CURLcode code = curl_easy_perform(handle);
		//Then from the result we can do error handling
		if (code != CURLcode::CURLE_OK) {
			ERR_PRINT(String("Could not make request to: ") + url + String(" error: ") + itos(code));
			return Vector<uint8_t>();
		}

		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, outStatus);
		curl_slist_free_all(curlHeaders);
		return buffer;
	}

	static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
		auto *vectorBuffer = reinterpret_cast<Vector<uint8_t>*>(userp);
		char *contentCStr = reinterpret_cast<char *>(contents);
		size_t realSize = size * nmemb;

		for (size_t i = 0; i < realSize; i++) {
			vectorBuffer->push_back(contentCStr[i]);
		}
	
		return realSize;
	}

	int32_t get_available_handle_index() {
		for (int32_t i = 0; i < this->m_activeHandles.size(); i++)
		{
			const RequestHandle_t &handle = this->m_activeHandles.at(i);
			if (handle.available) {
				return i;
			}
		}
		RequestHandle_t handle{};
		handle.easy_init();
		this->m_activeHandles.push_back(handle);
		return this->m_activeHandles.size() - 1;
	}

	//Have a thread pool for some batches of requests
	BRThreadPool m_threadPool;
	std::vector<RequestHandle_t> m_activeHandles;
};

#endif // HIGH_LEVEL_HTTP_CLIENT
