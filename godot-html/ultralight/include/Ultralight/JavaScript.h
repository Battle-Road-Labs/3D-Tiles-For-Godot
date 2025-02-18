/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <JavaScriptCore/JavaScript.h>
#include <Ultralight/Defines.h>
#include <Ultralight/RefPtr.h>
#include <Ultralight/String.h>

namespace ultralight {

///
/// This class wraps a JSContextRef (a JavaScript execution context for use with JavaScriptCore)
/// and locks the context on the current thread for the duration of its lifetime.
///
class UExport JSContext : public RefCounted {
public:
  /// Get the underlying JSContextRef for use with JavaScriptCore C API
  virtual JSContextRef ctx() = 0;

  /// Typecast to a JSContextRef for use with JavaScriptCore C API
  operator JSContextRef();

protected:
  virtual ~JSContext();
};

}  // namespace ultralight
