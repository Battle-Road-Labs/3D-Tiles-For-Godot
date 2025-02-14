/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_BITMAP_H
#define ULTRALIGHT_CAPI_BITMAP_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Bitmap
 *****************************************************************************/

///
/// Create empty bitmap.
///
ULExport ULBitmap ulCreateEmptyBitmap();

///
/// Create bitmap with certain dimensions and pixel format.
///
ULExport ULBitmap ulCreateBitmap(unsigned int width, unsigned int height, ULBitmapFormat format);

///
/// Create bitmap from existing pixel buffer. @see Bitmap for help using this function.
///
ULExport ULBitmap ulCreateBitmapFromPixels(unsigned int width, unsigned int height,
                                           ULBitmapFormat format, unsigned int row_bytes,
                                           const void* pixels, size_t size, bool should_copy);

///
/// Create bitmap from copy.
///
ULExport ULBitmap ulCreateBitmapFromCopy(ULBitmap existing_bitmap);

///
/// Destroy a bitmap (you should only destroy Bitmaps you have explicitly created via one of the
/// creation functions above.
///
ULExport void ulDestroyBitmap(ULBitmap bitmap);

///
/// Get the width in pixels.
///
ULExport unsigned int ulBitmapGetWidth(ULBitmap bitmap);

///
/// Get the height in pixels.
///
ULExport unsigned int ulBitmapGetHeight(ULBitmap bitmap);

///
/// Get the pixel format.
///
ULExport ULBitmapFormat ulBitmapGetFormat(ULBitmap bitmap);

///
/// Get the bytes per pixel.
///
ULExport unsigned int ulBitmapGetBpp(ULBitmap bitmap);

///
/// Get the number of bytes per row.
///
ULExport unsigned int ulBitmapGetRowBytes(ULBitmap bitmap);

///
/// Get the size in bytes of the underlying pixel buffer.
///
ULExport size_t ulBitmapGetSize(ULBitmap bitmap);

///
/// Whether or not this bitmap owns its own pixel buffer.
///
ULExport bool ulBitmapOwnsPixels(ULBitmap bitmap);

///
/// Lock pixels for reading/writing, returns pointer to pixel buffer.
///
ULExport void* ulBitmapLockPixels(ULBitmap bitmap);

///
/// Unlock pixels after locking.
///
ULExport void ulBitmapUnlockPixels(ULBitmap bitmap);

///
/// Get raw pixel buffer-- you should only call this if Bitmap is already locked.
///
ULExport void* ulBitmapRawPixels(ULBitmap bitmap);

///
/// Whether or not this bitmap is empty.
///
ULExport bool ulBitmapIsEmpty(ULBitmap bitmap);

///
/// Reset bitmap pixels to 0.
///
ULExport void ulBitmapErase(ULBitmap bitmap);

///
/// Write bitmap to a PNG on disk.
///
ULExport bool ulBitmapWritePNG(ULBitmap bitmap, const char* path);

///
/// This converts a BGRA bitmap to RGBA bitmap and vice-versa by swapping the red and blue channels.
///
ULExport void ulBitmapSwapRedBlueChannels(ULBitmap bitmap);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_BITMAP_H