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
/// User-defined clipboard interface.
///
/// The library uses this to read and write data to the system's clipboard.
///
/// AppCore automatically provides a platform-specific implementation of this that cuts, copies,
/// and pastes to the OS clipboard when you call App::Create().
///
/// If you are using Renderer::Create() instead of App::Create(), you will need to provide your own
/// implementation of this. @see Platform::set_clipboard().
///
class UExport Clipboard {
 public:
  virtual ~Clipboard();

  ///
  /// Clear the clipboard.
  ///
  virtual void Clear() = 0;

  ///
  /// Read plain text from the clipboard
  /// 
  /// This is called when the library wants to read text from the OS clipboard.
  ///
  virtual String ReadPlainText() = 0;

  ///
  /// Write plain text to the clipboard.
  /// 
  /// This is called when the library wants to write text to the OS clipboard.
  ///
  virtual void WritePlainText(const String& text) = 0;
};

} // namespace ultralight
