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
#include <Ultralight/Buffer.h>

namespace ultralight {

///
/// User-defined file system interface.
///
/// The library uses this to load all file URLs (eg, <file:///page.html>).
///
/// You can provide the library with your own FileSystem implementation so that file assets are
/// loaded from your own pipeline.
/// 
/// ## Usage
///
/// To provide your own custom FileSystem implementation, you should inherit from this class,
/// handle the virtual member functions, and then pass an instance of your class to
/// Platform::set_file_system() before calling Renderer::Create() or App::Create().
/// 
/// @note
/// \parblock
/// AppCore uses a default OS-specific FileSystem implementation when you call App::Create().
///
/// If you are using Renderer::Create(), you can still use AppCore's implementation-- see the
/// helper functions defined in <AppCore/Platform.h>.
/// \endparblock
///
class UExport FileSystem {
 public:
  virtual ~FileSystem();

  ///
  /// Check if a file exists within the file system.
  /// 
  /// @param  file_path  Relative file path (the string following the file:/// prefix)
  /// 
  /// @return Returns whether or not a file exists at the path specified.
  ///
  virtual bool FileExists(const String& file_path) = 0;

  ///
  /// Get the mime-type of a file (eg "text/html").
  /// 
  /// This is usually determined by analyzing the file extension.
  /// 
  /// If a mime-type cannot be determined, this should return "application/unknown".
  /// 
  /// @param  file_path  Relative file path (the string following the file:/// prefix)
  ///
  /// @return Returns whether or not a file exists at the path specified.
  ///
  virtual String GetFileMimeType(const String& file_path) = 0;

  ///
  /// Get the charset / encoding of a file (eg "utf-8", "iso-8859-1").
  /// 
  /// @note This is only applicable for text-based files (eg, "text/html", "text/plain") and is
  ///       usually determined by analyzing the contents of the file.
  /// 
  /// @param  file_path  Relative file path (the string following the file:/// prefix)
  ///
  /// @return Returns the charset of the specified file. If a charset cannot be determined, a safe
  ///         default to return is "utf-8".
  /// 
  virtual String GetFileCharset(const String& file_path) = 0;

  ///
  /// Open a file for reading and map it to a Buffer.
  /// 
  /// To minimize copies, you should map the requested file into memory and use Buffer::Create()
  /// to wrap the data pointer (unmapping should be performed in the destruction callback).
  ///
  /// @note  
  /// \parblock
  /// File data addresses returned from this function should generally be aligned to 16-byte
  /// boundaries (the default alignment on most operating systems-- if you're using C stdlib or
  /// C++ STL functions this is already handled for you).
  /// 
  /// This requirement is currently necessary when loading the ICU data file (eg, icudt67l.dat),
  /// and may be relaxed for  other files (but you may still see a performance benefit due to cache
  /// line alignment).
  /// 
  /// If you can't guarantee alignment or are unsure, you can use Buffer::CreateFromCopy to copy
  /// the file data content to an aligned block (at the expense of data duplication).
  /// \endparblock
  /// 
  /// @param  file_path  Relative file path (the string following the file:/// prefix)
  /// 
  /// @return If the file was able to be opened, this returns a Buffer object representing the
  ///         contents of the file. If the file was unable to be opened, you should return nullptr.
  /// 
  virtual RefPtr<Buffer> OpenFile(const String& file_path) = 0;
};

} // namespace ultralight
