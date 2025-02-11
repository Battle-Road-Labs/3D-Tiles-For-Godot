#ifndef LOCAL_ASSET_ACCESSOR_H
#define LOCAL_ASSET_ACCESSOR_H

#include "CesiumAsync/AsyncSystem.h"
#include "CesiumAsync/Promise.h"
#include "missing_functions.hpp"
#include <cstdint>
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/variant/packed_byte_array.hpp"
//#include <godot_cpp/core/error_macros.hpp>
#elif defined(CESIUM_GD_MODULE)
#include "core/error/error_list.h"
#include "core/io/file_access.h"
#endif

#include "CesiumAsync/IAssetAccessor.h"
#include "CesiumAsync/IAssetRequest.h"
#include "CesiumAsync/Future.h"
#include <memory>
#include "../Models/LocalAssetResponse.h"
#include "../Models/LocalAssetRequest.h"
#include "CesiumUtility/Uri.h"


constexpr uint16_t OK_STATUS = 200;

constexpr std::string_view FILE_ACCESS_ERR = "Could not read the given 3D Tileset!";

class LocalAssetAccessor final : public CesiumAsync::IAssetAccessor {
public:

	CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>>
		get(const CesiumAsync::AsyncSystem& asyncSystem,
			const std::string& url,
			const std::vector<THeader>& headers = {}) override {
		//Get the asset from the local file system instead of a web request
		return asyncSystem.createFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>([=](CesiumAsync::Promise<std::shared_ptr<CesiumAsync::IAssetRequest>> p_promise) {
			Error err;
			String pathToSearch;
			std::string contentType;
			contentType.reserve(50);

			if (m_relativeStartPath.is_empty()) {
				contentType = "application/json";
				pathToSearch = url.c_str();
				constexpr int32_t offsetToKeepSlash = 1;
				m_relativeStartPath = pathToSearch.substr(0, pathToSearch.rfind("/") + offsetToKeepSlash);
			}
			else {
				contentType = "application/octet-stream";
				pathToSearch = m_relativeStartPath + url.c_str();
			}

			const String godotFriendlyPath = pathToSearch.substr(strlen("file:///"));
			Ref<FileAccess> assetRef = open_file_access_with_err(godotFriendlyPath, FileAccess::READ, &err);

			if (err != Error::OK) {
				std::exception exc(FILE_ACCESS_ERR.data());
				return p_promise.reject(&exc);
			}


Vector<uint8_t> data;
#if defined(CESIUM_GD_EXT)
			PackedByteArray internalBuffer = assetRef->get_buffer(assetRef->get_length());
			
			data.resize(internalBuffer.size());
			//I really really wanted to avoid a memcpy... tough luck
			//TODO: Change internal holding type of LocalAssetResponse to a PackedByteArray
			memcpy(data.ptrw(), internalBuffer.ptr(), internalBuffer.size());
#elif defined(CESIUM_GD_MODULE)
			data = assetRef->get_buffer(assetRef->get_length());
#endif

			CesiumAsync::HttpHeaders headers;
			headers.insert({ "content-type", contentType });

			auto assetResponse = std::make_unique<LocalAssetResponse>(
				OK_STATUS,
				contentType,
				headers,
				data
			);
			constexpr size_t resOffset = 6;
			//std::string acceptedUri = "file:///" + CesiumUtility::Uri::uriPathToNativePath(url.substr(resOffset));
			auto assetRequest = std::make_shared<LocalAssetRequest>(
				"GET",
				url,
				headers,
				std::move(assetResponse)
			);

			return p_promise.resolve(assetRequest);
		});
	}

	CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> request(
		const CesiumAsync::AsyncSystem& asyncSystem,
		const std::string& verb,
		const std::string& url,
		const std::vector<THeader>& headers = std::vector<THeader>(),
		const std::span<const std::byte>& contentPayload = {}) override {

		return asyncSystem.createFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>([=](CesiumAsync::Promise<std::shared_ptr<CesiumAsync::IAssetRequest>> p_promise) {
			std::exception notImplemented("Method not implemented yet!");
			return p_promise.reject(&notImplemented);
		});
	}

	void tick() noexcept override {
		(void)0;
	}

private:
	String m_relativeStartPath;
};

#endif // !LOCAL_ASSET_ACCESSOR_H
