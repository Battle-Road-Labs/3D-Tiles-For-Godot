#ifndef __ULTRALIGHT_EXPORTS_H__
#define __ULTRALIGHT_EXPORTS_H__

#if defined(__WIN32__) || defined(_WIN32)
#  if defined(ULTRALIGHT_STATIC_BUILD)
#    define UExport 
#  else
#    if defined(ULTRALIGHT_IMPLEMENTATION)
#      define UExport __declspec(dllexport)
#    else
#      define UExport __declspec(dllimport)
#    endif
#  endif
#else
#  if defined(ULTRALIGHT_STATIC_BUILD)
#    define UExport 
#  else
#    define UExport __attribute__((visibility("default")))
#  endif
#endif

// UCExport is for symbols that are exported strictly by UltralightCore
#if defined(__WIN32__) || defined(_WIN32)
#  if defined(ULTRALIGHT_STATIC_BUILD)
#    define UCExport 
#  else
#    if defined(ULTRALIGHT_IMPLEMENTATION) && defined(ULTRALIGHT_MODULE_ULTRALIGHTCORE)
#      define UCExport __declspec(dllexport)
#    else
#      define UCExport __declspec(dllimport)
#    endif
#  endif
#else
#  if defined(ULTRALIGHT_STATIC_BUILD)
#    define UCExport 
#  else
#    define UCExport __attribute__((visibility("default")))
#  endif
#endif

#endif // __ULTRALIGHT_EXPORTS_H__