/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/

///
/// @file CAPI_Platform.h
/// 
/// Global platform singleton, manages user-defined platform handlers.
///
/// The library uses the Platform API for most platform-specific operations (eg, file access,
/// clipboard, font loading, GPU access, etc.).
///
/// @par Overview of which platform handlers are required / optional / provided:
///
/// |                   | ulCreateRenderer() | ulCreateApp() |
/// |-------------------|--------------------|---------------|
/// | FileSystem        | **Required**       | *Provided*    |
/// | FontLoader        | **Required**       | *Provided*    |
/// | Clipboard         |  *Optional*        | *Provided*    |
/// | GPUDriver         |  *Optional*        | *Provided*    |
/// | Logger            |  *Optional*        | *Provided*    |
/// | SurfaceDefinition |  *Provided*        | *Provided*    |
///
/// @note  This singleton should be set up before creating the Renderer or App.
///
#ifndef ULTRALIGHT_CAPI_PLATFORM_H
#define ULTRALIGHT_CAPI_PLATFORM_H

#include <Ultralight/CAPI/CAPI_Defines.h>
#include <Ultralight/CAPI/CAPI_Logger.h>
#include <Ultralight/CAPI/CAPI_FileSystem.h>
#include <Ultralight/CAPI/CAPI_FontLoader.h>
#include <Ultralight/CAPI/CAPI_Surface.h>
#include <Ultralight/CAPI/CAPI_GPUDriver.h>
#include <Ultralight/CAPI/CAPI_Clipboard.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Platform
 *****************************************************************************/

///
/// Set a custom Logger implementation.
///
/// This is used to log debug messages to the console or to a log file.
///
/// You should call this before ulCreateRenderer() or ulCreateApp().
///
/// @note  ulCreateApp() will use the default logger if you never call this.
///
/// @note  If you're not using ulCreateApp(), (eg, using ulCreateRenderer()) you can still use the
///        default logger by calling ulEnableDefaultLogger() (@see <AppCore/CAPI.h>)
///
ULExport void ulPlatformSetLogger(ULLogger logger);

///
/// Set a custom FileSystem implementation.
///
/// The library uses this to load all file URLs (eg, <file:///page.html>).
///
/// You can provide the library with your own FileSystem implementation so that file assets are
/// loaded from your own pipeline.
///
/// You should call this before ulCreateRenderer() or ulCreateApp().
/// 
/// @warning This is required to be defined before calling ulCreateRenderer()
///
/// @note  ulCreateApp() will use the default platform file system if you never call this.
///
/// @note  If you're not using ulCreateApp(), (eg, using ulCreateRenderer()) you can still use the
///        default platform file system by calling ulEnablePlatformFileSystem()'
///        (@see <AppCore/CAPI.h>)
///
ULExport void ulPlatformSetFileSystem(ULFileSystem file_system);

///
/// Set a custom FontLoader implementation.
/// 
/// The library uses this to load all system fonts.
///
/// Every operating system has its own library of installed system fonts. The FontLoader interface
/// is used to lookup these fonts and fetch the actual font data (raw TTF/OTF file data) for a given
/// given font description.
/// 
/// You should call this before ulCreateRenderer() or ulCreateApp().
/// 
/// @warning This is required to be defined before calling ulCreateRenderer()
/// 
/// @note  ulCreateApp() will use the default platform font loader if you never call this.
///
/// @note  If you're not using ulCreateApp(), (eg, using ulCreateRenderer()) you can still use the
///        default platform font loader by calling ulEnablePlatformFontLoader()'
///        (@see <AppCore/CAPI.h>)
///
ULExport void ulPlatformSetFontLoader(ULFontLoader font_loader);

///
/// Set a custom Surface implementation.
///
/// This can be used to wrap a platform-specific GPU texture, Windows DIB, macOS CGImage, or any
/// other pixel buffer target for display on screen.
///
/// By default, the library uses a bitmap surface for all surfaces but you can override this by
/// providing your own surface definition here.
///
/// You should call this before ulCreateRenderer() or ulCreateApp().
///
ULExport void ulPlatformSetSurfaceDefinition(ULSurfaceDefinition surface_definition);

///
/// Set a custom GPUDriver implementation.
///
/// This should be used if you have enabled the GPU renderer in the Config and are using
/// ulCreateRenderer() (which does not provide its own GPUDriver implementation).
///
/// The GPUDriver interface is used by the library to dispatch GPU calls to your native GPU context
/// (eg, D3D11, Metal, OpenGL, Vulkan, etc.) There are reference implementations for this interface
/// in the AppCore repo.
///
/// You should call this before ulCreateRenderer().
///
ULExport void ulPlatformSetGPUDriver(ULGPUDriver gpu_driver);

///
/// Set a custom Clipboard implementation.
///
/// This should be used if you are using ulCreateRenderer() (which does not provide its own
/// clipboard implementation).
///
/// The Clipboard interface is used by the library to make calls to the system's native clipboard
/// (eg, cut, copy, paste).
///
/// You should call this before ulCreateRenderer().
///
ULExport void ulPlatformSetClipboard(ULClipboard clipboard);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_PLATFORM_H