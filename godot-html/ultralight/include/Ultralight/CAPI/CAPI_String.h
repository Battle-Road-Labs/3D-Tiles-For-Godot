/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_STRING_H
#define ULTRALIGHT_CAPI_STRING_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * String
 *****************************************************************************/

///
/// Create string from null-terminated ASCII C-string.
///
ULExport ULString ulCreateString(const char* str);

///
/// Create string from UTF-8 buffer.
///
ULExport ULString ulCreateStringUTF8(const char* str, size_t len);

///
/// Create string from UTF-16 buffer.
///
ULExport ULString ulCreateStringUTF16(ULChar16* str, size_t len);

///
/// Create string from copy of existing string.
///
ULExport ULString ulCreateStringFromCopy(ULString str);

///
/// Destroy string (you should destroy any strings you explicitly Create).
///
ULExport void ulDestroyString(ULString str);

///
/// Get native UTF-8 buffer data (always null-terminated).
///
ULExport char* ulStringGetData(ULString str);

///
/// Get length (in bytes) of the UTF-8 buffer data, not including null terminator.
///
ULExport size_t ulStringGetLength(ULString str);

///
/// Whether this string is empty or not.
///
ULExport bool ulStringIsEmpty(ULString str);

///
/// Replaces the contents of 'str' with the contents of 'new_str'
///
ULExport void ulStringAssignString(ULString str, ULString new_str);

///
/// Replaces the contents of 'str' with the contents of a C-string.
///
ULExport void ulStringAssignCString(ULString str, const char* c_str);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_STRING_H