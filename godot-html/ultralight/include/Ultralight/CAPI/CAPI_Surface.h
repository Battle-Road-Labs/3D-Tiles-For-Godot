/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_SURFACE_H
#define ULTRALIGHT_CAPI_SURFACE_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Surface
 *****************************************************************************/

///
/// Width (in pixels).
///
ULExport unsigned int ulSurfaceGetWidth(ULSurface surface);

///
/// Height (in pixels).
///
ULExport unsigned int ulSurfaceGetHeight(ULSurface surface);

///
/// Number of bytes between rows (usually width * 4)
///
ULExport unsigned int ulSurfaceGetRowBytes(ULSurface surface);

///
/// Size in bytes.
///
ULExport size_t ulSurfaceGetSize(ULSurface surface);

///
/// Lock the pixel buffer and get a pointer to the beginning of the data for reading/writing.
///
/// Native pixel format is premultiplied BGRA 32-bit (8 bits per channel).
///
ULExport void* ulSurfaceLockPixels(ULSurface surface);

///
/// Unlock the pixel buffer.
///
ULExport void ulSurfaceUnlockPixels(ULSurface surface);

///
/// Resize the pixel buffer to a certain width and height (both in pixels).
///
/// This should never be called while pixels are locked.
///
ULExport void ulSurfaceResize(ULSurface surface, unsigned int width, unsigned int height);

///
/// Set the dirty bounds to a certain value.
///
/// This is called after the Renderer paints to an area of the pixel buffer. (The new value will be
/// joined with the existing dirty_bounds())
///
ULExport void ulSurfaceSetDirtyBounds(ULSurface surface, ULIntRect bounds);

///
/// Get the dirty bounds.
///
/// This value can be used to determine which portion of the pixel buffer has been updated since the
/// last call to ulSurfaceClearDirtyBounds().
///
/// The general algorithm to determine if a Surface needs display is:
/// <pre>
///   if (!ulIntRectIsEmpty(ulSurfaceGetDirtyBounds(surface))) {
///       // Surface pixels are dirty and needs display.
///       // Cast Surface to native Surface and use it here (pseudo code)
///       DisplaySurface(surface);
///
///       // Once you're done, clear the dirty bounds:
///       ulSurfaceClearDirtyBounds(surface);
///  }
///  </pre>
///
ULExport ULIntRect ulSurfaceGetDirtyBounds(ULSurface surface);

///
/// Clear the dirty bounds.
///
/// You should call this after you're done displaying the Surface.
///
ULExport void ulSurfaceClearDirtyBounds(ULSurface surface);

///
/// Get the underlying user data pointer (this is only valid if you have set a custom surface
/// implementation via ulPlatformSetSurfaceDefinition).
///
/// This will return nullptr if this surface is the default ULBitmapSurface.
///
ULExport void* ulSurfaceGetUserData(ULSurface surface);

/******************************************************************************
 * BitmapSurface
 *****************************************************************************/

///
/// Get the underlying Bitmap from the default Surface.
///
/// @note  Do not call ulDestroyBitmap() on the returned value, it is owned by the surface.
///
ULExport ULBitmap ulBitmapSurfaceGetBitmap(ULBitmapSurface surface);

/******************************************************************************
 * Surface Definition
 *****************************************************************************/

///
/// The callback invoked when a Surface is created.
///
/// @param  width  The width in pixels.
/// @param  height  The height in pixels.
///
/// @return  This callback should return a pointer to user-defined data for the instance. This user
///          data pointer will be passed to all other callbacks when operating on the instance.
///
typedef void* (*ULSurfaceDefinitionCreateCallback)(unsigned int width, unsigned int height);

///
/// The callback invoked when a Surface is destroyed.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef void (*ULSurfaceDefinitionDestroyCallback)(void* user_data);

///
/// The callback invoked when a Surface's width (in pixels) is requested.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef unsigned int (*ULSurfaceDefinitionGetWidthCallback)(void* user_data);

///
/// The callback invoked when a Surface's height (in pixels) is requested.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef unsigned int (*ULSurfaceDefinitionGetHeightCallback)(void* user_data);

///
/// The callback invoked when a Surface's row bytes is requested.
///
/// @note  This value is also known as "stride". Usually width * 4.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef unsigned int (*ULSurfaceDefinitionGetRowBytesCallback)(void* user_data);

///
/// The callback invoked when a Surface's size (in bytes) is requested.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef size_t (*ULSurfaceDefinitionGetSizeCallback)(void* user_data);

///
/// The callback invoked when a Surface's pixel buffer is requested to be locked for reading/writing
/// (should return a pointer to locked bytes).
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef void* (*ULSurfaceDefinitionLockPixelsCallback)(void* user_data);

///
/// The callback invoked when a Surface's pixel buffer is requested to be unlocked after previously
/// being locked.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
typedef void (*ULSurfaceDefinitionUnlockPixelsCallback)(void* user_data);

///
/// The callback invoked when a Surface is requested to be resized to a certain width/height.
///
/// @param  user_data  User data pointer uniquely identifying the surface.
///
/// @param  width      Width in pixels.
///
/// @param  height     Height in pixels.
///
typedef void (*ULSurfaceDefinitionResizeCallback)(void* user_data, unsigned int width,
                                                  unsigned int height);

typedef struct {
  ULSurfaceDefinitionCreateCallback create;
  ULSurfaceDefinitionDestroyCallback destroy;
  ULSurfaceDefinitionGetWidthCallback get_width;
  ULSurfaceDefinitionGetHeightCallback get_height;
  ULSurfaceDefinitionGetRowBytesCallback get_row_bytes;
  ULSurfaceDefinitionGetSizeCallback get_size;
  ULSurfaceDefinitionLockPixelsCallback lock_pixels;
  ULSurfaceDefinitionUnlockPixelsCallback unlock_pixels;
  ULSurfaceDefinitionResizeCallback resize;
} ULSurfaceDefinition;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_SURFACE_H