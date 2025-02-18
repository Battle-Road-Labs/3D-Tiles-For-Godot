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

class String8;
class String16;

///
/// @brief  A UTF-32 string container.
///
class UExport String32 {
public:
  // Make an empty String32
  String32();

  // Make a String32 from raw UTF-32 string with certain length
  String32(const char32_t* c_str, size_t len);

  // Make a deep copy of String32
  String32(const String32& other);

  ~String32();

  // Assign a String32 to this one, deep copy is made
  String32& operator=(const String32& other);

  // Append a String32 to this one.
  String32& operator+=(const String32& other);

  // Concatenation operator
  inline friend String32 operator+(String32 lhs, const String32& rhs) { lhs += rhs; return lhs; }

  // Get raw UTF-32 data
  char32_t* data() { return data_; }

  // Get raw UTF-32 data (const)
  const char32_t* data() const { return data_; }

  // Get length in characters.
  size_t length() const { return length_; }

  // Get size in characters (synonym for length)
  size_t size() const { return length_; }

  // Get size in bytes
  size_t sizeBytes() const { return length_ * sizeof(char32_t); }

  // Check if string is empty.
  bool empty() const { return !data_ || length_ == 0; }

  // Get character at specific position
  char32_t& operator[](size_t pos) { return data_[pos]; }

  // Get character at specific position (const)
  const char32_t& operator[](size_t pos) const { return data_[pos]; }

  // Get a UTF-8 copy of this string
  String8 utf8() const;
  
  // Get a UTF-16 copy of this string
  String16 utf16() const;

private:
  char32_t* data_;
  size_t length_;
};

}  // namespace ultralight
