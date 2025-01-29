#ifndef LOCAL_ASSET_RESPONSE_H
#define LOCAL_ASSET_RESPONSE_H

#include <CesiumAsync/IAssetResponse.h>
#include <cstdint>
#include <vector>
#if defined(CESIUM_GD_EXT)
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/templates/vector.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/variant/variant.h"
#endif

class LocalAssetResponse : public CesiumAsync::IAssetResponse {
public:
	LocalAssetResponse(
		uint16_t statusCode,
		const std::string& contentType,
		const CesiumAsync::HttpHeaders& headers,
		const Vector<uint8_t>& data)
		: m_statusCode{ statusCode },
		m_contentType{ contentType },
		m_headers{ headers },
		m_data{ data } {}

	/**
	* @brief Returns the HTTP response code.
	*/
	uint16_t statusCode() const override {
		return this->m_statusCode;
	}

	/**
	 * @brief Returns the HTTP content type
	 */
	std::string contentType() const override {
		return this->m_contentType;
	}

	/**
	 * @brief Returns the HTTP headers of the response
	 */
	const CesiumAsync::HttpHeaders& headers() const override {
		return this->m_headers;
	}

	/**
	 * @brief Returns the data of this response
	 */
	gsl::span<const std::byte> data() const override {
		const std::byte* bytePtr = reinterpret_cast<const std::byte*>(this->m_data.ptr());
		return gsl::span<const std::byte>(bytePtr, bytePtr + this->m_data.size());
	}

private:
	uint16_t m_statusCode;
	std::string m_contentType;
	CesiumAsync::HttpHeaders m_headers;
	Vector<uint8_t> m_data;
};

#endif // !LOCAL_ASSET_RESPONSE_H
