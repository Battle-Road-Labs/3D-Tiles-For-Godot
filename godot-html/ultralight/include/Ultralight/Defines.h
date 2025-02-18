/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once

// Needed for limit defines, like INTMAX_MAX, which is used by the std C++ library
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <cstdint>
#include <cstddef>
#include <limits.h>

#ifdef SWIG
#define UExport
#else

#include "Exports.h"

// Require C++11 Support
#if defined(_MSC_VER)
#  if _MSC_VER < 1800 
#    error This project needs at least Visual Studio 2013 to build
#  endif
#elif __cplusplus <= 199711L
#  error This project can only be compiled with a compiler that supports C++11
#endif

#if INTPTR_MAX == INT32_MAX
#  define UL_ARCH_32_BIT
#elif INTPTR_MAX == INT64_MAX
#  define UL_ARCH_64_BIT
#else
#  error "Unknown CPU architecture: environment not 32 or 64-bit."
#endif

#if defined(__aarch64__)
#  define UL_ARCH_ARM64
#  if defined(__APPLE__)
#    define UL_ARCH_ARM64_APPLE_SILICON
#  endif
#endif

#if defined(__WIN32__) || defined(_WIN32)
#  define _thread_local __declspec(thread)
#  ifndef _NATIVE_WCHAR_T_DEFINED
#    define DISABLE_NATIVE_WCHAR_T
#  endif
#else
#  define _thread_local __thread
#endif

#ifndef UL_COMPILER_GCC_LIKE
#  define UL_COMPILER_GCC_LIKE (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__))
#endif

#ifndef UL_ALWAYS_INLINE
#if defined(UL_COMPILER_GCC_LIKE) && defined(NDEBUG)
#    define UL_ALWAYS_INLINE inline __attribute__((__always_inline__))
#  elif defined(_MSC_VER) && defined(NDEBUG)
#    define UL_ALWAYS_INLINE __forceinline
#  else
#    define UL_ALWAYS_INLINE inline
#  endif
#endif

#ifndef UL_UNLIKELY
#  if defined(UL_COMPILER_GCC_LIKE)
#    define UL_UNLIKELY(x) __builtin_expect(!!(x), 0)
#  else
#    define UL_UNLIKELY(x) (x)
#  endif
#endif

#endif

#define ULTRALIGHT_VERSION "1.4.0"
#define ULTRALIGHT_VERSION_MAJOR 1
#define ULTRALIGHT_VERSION_MINOR 4
#define ULTRALIGHT_VERSION_PATCH 0

#define WEBKIT_VERSION "615.1.18.100.1"
#define WEBKIT_VERSION_MAJOR 615
#define WEBKIT_VERSION_MINOR 1
#define WEBKIT_VERSION_TINY 18
#define WEBKIT_VERSION_MICRO 100
#define WEBKIT_VERSION_NANO 1

#ifdef __cplusplus
extern "C" {
#endif

///
/// Full library version string (corresponds to ULTRALIGHT_VERSION)
/// 
UExport const char* UltralightVersionString();

UExport uint32_t UltralightVersionMajor();
UExport uint32_t UltralightVersionMinor();
UExport uint32_t UltralightVersionPatch();

///
/// Full WebKit version string (corresponds to WEBKIT_VERSION)
/// 
UExport const char* WebKitVersionString();

UExport uint32_t WebKitVersionMajor();
UExport uint32_t WebKitVersionMinor();
UExport uint32_t WebKitVersionTiny();
UExport uint32_t WebKitVersionMicro();
UExport uint32_t WebKitVersionNano();

#ifdef __cplusplus
}
#endif