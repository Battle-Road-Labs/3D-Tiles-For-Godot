#ifndef NETWORK_ASSET_ACCESSOR_H
#define NETWORK_ASSET_ACCESSOR_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/http_request.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include <scene/main/http_request.h>

#endif

#include <CesiumAsync/IAssetAccessor.h>

// Use Godot's native HTTPClient on macOS for better compatibility
// (curl has DNS/SSL issues with macOS's network stack).
// Also use it on web, because curl/ssl are not linked in Emscripten builds
// (web networking goes through browser fetch via Godot's HTTPClient).
#if defined(__APPLE__) || defined(__EMSCRIPTEN__)
#include "../Utils/GodotHttpClient.h"
#else
#include "../Utils/CurlHttpClient.h"
#endif

class Cesium3DTileset;
class CesiumHTTPRequestNode;

class NetworkAssetAccessor final : public CesiumAsync::IAssetAccessor {

public:
	NetworkAssetAccessor();

	CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
		get(const CesiumAsync::AsyncSystem& asyncSystem,
			const std::string& url,
			const std::vector<THeader>& headers = {}) override;

	CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> request(
		const CesiumAsync::AsyncSystem& asyncSystem,
		const std::string& verb,
		const std::string& url,
		const std::vector<THeader>& headers = std::vector<THeader>(),
		const std::span<const std::byte>& contentPayload = {}) override;

	void tick() noexcept override;

private:
	CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> process_request(HTTPClient::Method method, const CesiumAsync::AsyncSystem &asyncSystem, const std::string &url, const std::vector<THeader> &headers = {});

#if defined(__APPLE__) || defined(__EMSCRIPTEN__)
	GodotHttpClient<100> m_httpClient{};
#else
	CurlHttpClient<100> m_httpClient{};
#endif
};

#endif
