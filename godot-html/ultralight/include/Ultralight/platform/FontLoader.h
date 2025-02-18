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
/// Represents a font file, either on-disk path or in-memory file contents.
///
class UExport FontFile : public RefCounted {
 public:
  ///
  /// Create a font file from an on-disk file path.
  ///
  /// @note  The file path should already exist.
  ///
  static RefPtr<FontFile> Create(const String& filepath);

  ///
  /// Create a font file from an in-memory buffer.
  ///
  static RefPtr<FontFile> Create(RefPtr<Buffer> buffer);

  ///
  /// Whether or not this font file was created from an in-memory buffer.
  ///
  virtual bool is_in_memory() const = 0;

  ///
  /// The file path (if any).
  ///
  virtual String filepath() const = 0;

  ///
  /// The in-memory buffer (if any).
  ///
  virtual RefPtr<Buffer> buffer() const = 0;

  ///
  /// Unique hash (if this is a filepath, only the path string is hashed).
  ///
  virtual uint32_t hash() const = 0;

 protected:
  FontFile();
  virtual ~FontFile();
  FontFile(const FontFile&);
  void operator=(const FontFile&);
};

///
/// User-defined font loader interface.
/// 
/// The library uses this to load all system fonts.
///
/// Every operating system has its own library of installed system fonts. The FontLoader interface
/// is used to lookup these fonts and fetch the actual font data (raw TTF/OTF file data) for a given
/// given font description.
/// 
/// ## Usage
/// 
/// To provide your own custom FontLoader implementation, you should inherit from this class,
/// handle the virtual member functions, and then pass an instance of your class to
/// Platform::set_font_loader() before calling Renderer::Create() or App::Create().
///
/// @note
/// \parblock
/// AppCore uses a default OS-specific FontLoader implementation when you call App::Create().
///
/// If you are using Renderer::Create(), you can still use AppCore's implementation-- see the
/// helper functions defined in <AppCore/Platform.h>.
/// \endparblock
///
class UExport FontLoader {
 public:
  virtual ~FontLoader();

  ///
  /// Fallback font family name. Will be used if all other fonts fail to load.
  ///
  /// @note  This font should be guaranteed to exist (eg, FontLoader::Load won't fail when passed
  ///        this font family name).
  ///
  virtual String fallback_font() const = 0;

  ///
  /// Fallback font family name that can render the specified characters. Mainly used to support
  /// CJK (Chinese, Japanese, Korean) text display.
  ///
  /// @param  characters  One or more UTF-16 characters. This is almost always a single character.
  ///
  /// @param  weight      Font weight.
  ///
  /// @param  italic      Whether or not italic is requested.
  ///
  /// @return  Returns a font family name that can render the text.
  ///
  virtual String fallback_font_for_characters(const String& characters, int weight,
                                              bool italic) const = 0;

  ///
  /// Get the actual font file data (TTF/OTF) for a given font description.
  ///
  /// @param  family  Font family name.
  ///
  /// @param  weight  Font weight.
  ///
  /// @param  italic  Whether or not italic is requested.
  ///
  /// @return  A font file matching the given description (either an on-disk font filepath or an
  ///          in-memory file contents). You can return NULL here and the loader will fallback to
  ///          another font.
  ///
  virtual RefPtr<FontFile> Load(const String& family, int weight, bool italic) = 0;
};

} // namespace ultralight
