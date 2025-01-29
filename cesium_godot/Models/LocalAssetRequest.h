#ifndef LOCAL_ASSET_REQUEST_H
#define LOCAL_ASSET_REQUEST_H
#include <CesiumAsync/IAssetRequest.h>
#include <memory>
#include "LocalAssetResponse.h"

class LocalAssetRequest : public CesiumAsync::IAssetRequest {
public:
	LocalAssetRequest(
		const std::string& method,
		const std::string& url,
		const CesiumAsync::HttpHeaders& headers,
		std::unique_ptr<LocalAssetResponse> pResponse)
		: m_requestMethod{ method },
		m_requestUrl{ url },
		m_requestHeaders{ headers },
		m_pResponse{ std::move(pResponse) } {}

	/**
   * @brief Gets the request's method. This method may be called from any
   * thread.
   */
	const std::string& method() const override {
		return this->m_requestMethod;
	}

	/**
	 * @brief Gets the requested URL. This method may be called from any thread.
	 */
	const std::string& url() const override {
		return this->m_requestUrl;
	}

	/**
	 * @brief Gets the request's header. This method may be called from any
	 * thread.
	 */
	const CesiumAsync::HttpHeaders& headers() const override {
		return this->m_requestHeaders;
	}

	/**
	 * @brief Gets the response, or nullptr if the request is still in progress.
	 * This method may be called from any thread.
	 */
	const CesiumAsync::IAssetResponse* response() const override {
		return this->m_pResponse.get();
	}

private:
	std::string m_requestMethod;
	std::string m_requestUrl;
	CesiumAsync::HttpHeaders m_requestHeaders;
	std::unique_ptr<LocalAssetResponse> m_pResponse;

};

#endif // !LOCAL_ASSET_REQUEST_H
