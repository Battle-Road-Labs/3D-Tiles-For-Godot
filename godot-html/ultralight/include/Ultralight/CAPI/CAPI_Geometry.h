/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_GEOMETRY_H
#define ULTRALIGHT_CAPI_GEOMETRY_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Rect
 *****************************************************************************/

///
/// Whether or not a ULRect is empty (all members equal to 0)
///
ULExport bool ulRectIsEmpty(ULRect rect);

///
/// Create an empty ULRect (all members equal to 0)
///
ULExport ULRect ulRectMakeEmpty();

/******************************************************************************
 * IntRect
 *****************************************************************************/

///
/// Whether or not a ULIntRect is empty (all members equal to 0)
///
ULExport bool ulIntRectIsEmpty(ULIntRect rect);

///
/// Create an empty ULIntRect (all members equal to 0)
///
ULExport ULIntRect ulIntRectMakeEmpty();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_GEOMETRY_H