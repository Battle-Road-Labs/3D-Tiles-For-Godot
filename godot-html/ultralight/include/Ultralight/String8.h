/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <stddef.h>

namespace ultralight {

class String16;
class String32;

///
/// @brief  A UTF-8 string container.
//
class UExport String8 {
public:
  // Make an empty String8
  String8();

  // Make a String8 from raw, null-terminated UTF-8 string
  String8(const char* c_str);

  // Make a String8 from raw UTF-8 string with certain length
  String8(const char* c_str, size_t len);

  // Make a deep copy of String8
  String8(const String8& other);

  ~String8();

  // Assign a String8 to this one, deep copy is made
  String8& operator=(const String8& other);

  // Append a String8 to this one.
  String8& operator+=(const String8& other);

  // Concatenation operator
  inline friend String8 operator+(String8 lhs, const String8& rhs) { lhs += rhs; return lhs; }

  // Get raw UTF-8 data
  char* data() { return data_; }

  // Get raw UTF-8 data (const)
  const char* data() const { return data_; }

  // Get length in characters.
  size_t length() const { return length_; }

  // Get size in characters (synonym for length)
  size_t size() const { return length_; }

  // Get size in bytes
  size_t sizeBytes() const { return length_ * sizeof(char); }

  // Check if string is empty.
  bool empty() const { return !data_ || length_ == 0; }

  // Get character at specific position
  char& operator[](size_t pos) { return data_[pos]; }

  // Get character at specific position (const)
  const char& operator[](size_t pos) const { return data_[pos]; }

  // Get a UTF-16 copy of this string
  String16 utf16() const;

  // Get a UTF-32 copy of this string
  String32 utf32() const;

private:
  char* data_;
  size_t length_;
};

}  // namespace ultralight
