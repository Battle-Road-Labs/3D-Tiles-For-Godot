/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2024 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/String.h>
#include <JavaScriptCore/JavaScript.h>

namespace ultralight {

///
/// MessageSource types
///
enum MessageSource {
  kMessageSource_XML = 0,
  kMessageSource_JS,
  kMessageSource_Network,
  kMessageSource_ConsoleAPI,
  kMessageSource_Storage,
  kMessageSource_AppCache,
  kMessageSource_Rendering,
  kMessageSource_CSS,
  kMessageSource_Security,
  kMessageSource_ContentBlocker,
  kMessageSource_Media,
  kMessageSource_MediaSource,
  kMessageSource_WebRTC,
  kMessageSource_ITPDebug,
  kMessageSource_PrivateClickMeasurement,
  kMessageSource_PaymentRequest,
  kMessageSource_Other,
};

enum MessageType {
  kMessageType_Log = 0,
  kMessageType_Dir,
  kMessageType_DirXML,
  kMessageType_Table,
  kMessageType_Trace,
  kMessageType_StartGroup,
  kMessageType_StartGroupCollapsed,
  kMessageType_EndGroup,
  kMessageType_Clear,
  kMessageType_Assert,
  kMessageType_Timing,
  kMessageType_Profile,
  kMessageType_ProfileEnd,
  kMessageType_Image,
};

///
/// MessageLevel types
///
enum MessageLevel {
  kMessageLevel_Log = 0,
  kMessageLevel_Warning,
  kMessageLevel_Error,
  kMessageLevel_Debug,
  kMessageLevel_Info,
};

///
/// @brief  Interface for console messages.
///
class UExport ConsoleMessage {
 public:
  ~ConsoleMessage() { }

  ///
  /// The source of the message.
  /// 
  virtual MessageSource source() const = 0;

  ///
  /// The type of content displayed.
  /// 
  virtual MessageType type() const = 0;

  ///
  /// The log level for the message.
  /// 
  virtual MessageLevel level() const = 0;

  ///
  /// The message as a string-- for multi-argument calls to console.log() this just converts the
  /// first parameter to a string.
  /// 
  virtual String message() const = 0;

  ///
  /// The line number of the JavaScript associated with this call, if any.
  /// 
  virtual uint32_t line_number() const = 0;

  ///
  /// The column number of the JavaScript associated with this call, if any.
  /// 
  virtual uint32_t column_number() const = 0;

  ///
  /// The source id (eg, URL) of the page associated with this call, if any.
  /// 
  virtual String source_id() const = 0;

  ///
  /// The JavaScript execution context for the arguments, if any.
  /// 
  virtual JSContextRef argument_context() const = 0;

  ///
  /// The number of JavaScript arguments passed to console.log(), if any.
  /// 
  virtual uint32_t num_arguments() const = 0;

  ///
  /// Get the JavaScript argument at a specific index (numbering starts at 0).
  /// 
  virtual JSValueRef argument_at(uint32_t idx) const = 0;
};

} // namespace ultralight
