/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/String.h>

namespace ultralight {

///
/// Log levels, used with Logger::LogMessage
///
enum class LogLevel : uint8_t {
  Error,
  Warning,
  Info
};

///
/// User-defined logging interface.
///          
/// The library uses this to display log messages for debugging during development.
///
/// This is intended to be implemented by users and defined before creating the Renderer. 
/// 
/// @see Platform::set_logger()
///
class UExport Logger {
public:
  virtual ~Logger();

  ///
  /// Called when the library wants to display a log message.
  ///
  virtual void LogMessage(LogLevel log_level, const String& message) = 0;
};

}  // namespace ultralight
