/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/RefPtr.h>
#include <stddef.h>

namespace ultralight {

class String8;
class String32;

namespace detail {
  template<int> struct selector;
  template<> struct selector<4> { typedef char16_t Char16; };
  template<> struct selector<2> { typedef wchar_t Char16; };
}

#ifdef DISABLE_NATIVE_WCHAR_T
// Force Char16 type to use char16_t, used on Windows when native wchar_t support is disabled.
typedef char16_t Char16;
#else
// We use wchar_t if size == 2, otherwise use char16_t
typedef detail::selector<sizeof(wchar_t)>::Char16 Char16;
#endif

///
/// @brief  A UTF-16 string container.
///
class UExport String16 {
public:
  // Make an empty String16
  String16();

  // Make a String16 from raw UTF-16 buffer with certain length
  String16(const Char16* str, size_t len);

  // Make a String16 from raw unsigned short UTF-16 buffer with certain length. Useful on Windows
  // when native support for wchar_t is disabled (eg, /Zc:wchar_t-).
  String16(const unsigned short* str, size_t len);

  // Make a deep copy of String16
  String16(const String16& other);

  ~String16();

  // Assign a String16 to this one, deep copy is made
  String16& operator=(const String16& other);

  // Append a String16 to this one.
  String16& operator+=(const String16& other);

  // Concatenation operator
  inline friend String16 operator+(String16 lhs, const String16& rhs) { lhs += rhs; return lhs; }

  // Get raw UTF-16 data
  Char16* data() { return data_; }

  // Get raw UTF-16 data (const)
  const Char16* data() const { return data_; }

  // Get raw UTF-16 data as unsigned short. This is useful on Windows if you compile without native
  // support for wchar_t (eg, /Zc:wchar_t-)
  unsigned short* udata() { return reinterpret_cast<unsigned short*>(data_); }

  // Get raw UTF-16 data as unsigned short (const).
  const unsigned short* udata() const { return reinterpret_cast<const unsigned short*>(data_); }

  // Get length in characters.
  size_t length() const { return length_; }

  // Get size in characters (synonym for length)
  size_t size() const { return length_; }

  // Get size in bytes
  size_t sizeBytes() const { return length_ * sizeof(Char16); }

  // Check if string is empty.
  bool empty() const { return !data_ || length_ == 0; }

  // Get character at specific position
  Char16& operator[](size_t pos) { return data_[pos]; }

  // Get character at specific position (const)
  const Char16& operator[](size_t pos) const { return data_[pos]; }

  // Get a UTF-8 copy of this string
  String8 utf8() const;

  // Get a UTF-32 copy of this string
  String32 utf32() const;

private:
  Char16* data_;
  size_t length_;
};

///
/// @brief  A UTF-16 string vector.
///
class UExport String16Vector : public RefCounted {
public:
  // Create an empty string vector
  static RefPtr<String16Vector> Create();

  // Create a string vector from an existing array (a deep copy is made)
  static RefPtr<String16Vector> Create(const String16* stringArray, size_t len);

  // Add an element to the back of the string vector
  virtual void push_back(const String16& val) = 0;

  // Get raw String16 vector array
  virtual String16* data() = 0;

  // Get the number of elements in vector
  virtual size_t size() const = 0;

protected:
  String16Vector();
  virtual ~String16Vector();
  String16Vector(const String16Vector&);
  void operator=(const String16Vector&);
};

}  // namespace ultralight
