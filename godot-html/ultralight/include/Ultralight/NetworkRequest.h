/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>

namespace ultralight {

///
/// @brief  Interface for Network requests.
///
class UExport NetworkRequest {
 public:
  virtual ~NetworkRequest() { }

  ///
  /// URL of the request.
  /// 
  virtual String url() const = 0;

  ///
  /// The host portion of the URL.
  /// 
  virtual String urlHost() const = 0;

  ///
  /// The protocol of the URL (eg, "http")
  /// 
  virtual String urlProtocol() const = 0;

  ///
  /// The HTTP method (eg, "POST" or "GET")
  /// 
  virtual String httpMethod() const = 0;

  ///
  /// The origin of the request.
  /// 
  virtual String httpOrigin() const = 0;

  ///
  /// The user-agent of the request.
  /// 
  virtual String httpUserAgent() const = 0;

  ///
  /// Enforce additional TLS/SSL certificate validation by verifying the
  /// server's pinned public key.
  ///
  /// The public key string can be any number of base64 encoded sha256 hashes
  /// preceded by "sha256//" and separated by ";".
  ///
  /// For more info see the cURL docs:
  ///   <https://curl.se/libcurl/c/CURLOPT_PINNEDPUBLICKEY.html>
  ///
  virtual void EnforcePinnedPublicKey(const String& public_key) = 0;
};

} // namespace ultralight
