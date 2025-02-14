/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/String8.h>
#include <Ultralight/String16.h>
#include <Ultralight/String32.h>

namespace ultralight {

///
/// @brief  UTF-8 String container with conversions for UTF-16 and UTF-32.
///
/// @note   Internally, all strings are represented as String8 (UTF-8).
///
class UExport String {
public:
  ///
  /// Create empty string
  ///
  String();

  ///
  /// Create from null-terminated, ASCII C-string
  ///
  String(const char* str);

  ///
  /// Create from raw, UTF-8 string with certain length
  ///
  String(const char* str, size_t len);

  ///
  /// Create from existing String8 (UTF-8).
  ///
  String(const String8& str);

  ///
  /// Create from raw UTF-16 string with certain length
  ///
  String(const Char16* str, size_t len);

  ///
  /// Create from existing String16 (UTF-16)
  ///
  String(const String16& str);

  ///
  /// Create from existing String32 (UTF-32)
  ///
  String(const String32& str);

  ///
  /// Copy constructor
  ///
  String(const String& other);

  ///
  /// Destructor
  ///
  ~String();

  ///
  /// Assign string from another, copy is made
  ///
  String& operator=(const String& other);

  ///
  /// Append string with another
  ///
  String& operator+=(const String& other);
  
  ///
  /// Concatenation operator
  ///
  inline friend String operator+(String lhs, const String& rhs) { lhs += rhs; return lhs; }

  ///
  /// Get native UTF-8 string
  ///
  String8& utf8() { return str_; }

  ///
  /// Get native UTF-8 string
  ///
  const String8& utf8() const { return str_; }

  ///
  /// Convert to UTF-16 string
  ///
  String16 utf16() const;

  ///
  /// Convert to UTF-32 string
  ///
  String32 utf32() const;

  ///
  /// Check if string is empty or not
  ///
  bool empty() const { return str_.empty(); }

private:
  String8 str_;
};


}  // namespace ultralight

