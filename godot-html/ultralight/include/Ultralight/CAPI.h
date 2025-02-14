/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_H
#define ULTRALIGHT_CAPI_H

/******************************************************************************
 * API Note:
 *
 * You should only destroy objects that you explicitly create. Do not destroy
 * any objects returned from the API or callbacks unless otherwise noted.
 *****************************************************************************/

#include <Ultralight/CAPI/CAPI_Defines.h>
#include <Ultralight/CAPI/CAPI_Bitmap.h>
#include <Ultralight/CAPI/CAPI_Buffer.h>
#include <Ultralight/CAPI/CAPI_Clipboard.h>
#include <Ultralight/CAPI/CAPI_Config.h>
#include <Ultralight/CAPI/CAPI_FileSystem.h>
#include <Ultralight/CAPI/CAPI_FontFile.h>
#include <Ultralight/CAPI/CAPI_FontLoader.h>
#include <Ultralight/CAPI/CAPI_Geometry.h>
#include <Ultralight/CAPI/CAPI_GPUDriver.h>
#include <Ultralight/CAPI/CAPI_KeyEvent.h>
#include <Ultralight/CAPI/CAPI_Logger.h>
#include <Ultralight/CAPI/CAPI_MouseEvent.h>
#include <Ultralight/CAPI/CAPI_Platform.h>
#include <Ultralight/CAPI/CAPI_Renderer.h>
#include <Ultralight/CAPI/CAPI_ScrollEvent.h>
#include <Ultralight/CAPI/CAPI_GamepadEvent.h>
#include <Ultralight/CAPI/CAPI_Session.h>
#include <Ultralight/CAPI/CAPI_String.h>
#include <Ultralight/CAPI/CAPI_Surface.h>
#include <Ultralight/CAPI/CAPI_View.h>

#endif // ULTRALIGHT_CAPI_H
