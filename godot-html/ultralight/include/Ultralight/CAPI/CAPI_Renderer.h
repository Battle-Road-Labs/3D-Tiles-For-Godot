/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/

///
/// @file CAPI_Renderer.h
///
/// Core renderer singleton for the library, coordinates all library functions.
///
/// The Renderer class is responsible for creating and painting \link CAPI_View.h Views \endlink,
/// managing \link CAPI_Session.h Sessions \endlink, as well as coordinating network requests,
/// events, JavaScript execution, and more.
///
/// ## Initializing the Renderer
/// 
/// To initialize the library, you should set up the \link CAPI_Platform.h Platform \endlink
/// singleton and call ulCreateRenderer().
/// 
/// @note If you'd like to let the library manage window creation you can instead call
///       ulCreateApp().
/// 
#ifndef ULTRALIGHT_CAPI_RENDERER_H
#define ULTRALIGHT_CAPI_RENDERER_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// Create the core renderer singleton for the library directly.
///
/// Unlike ulCreateApp(), this does not use any native windows for drawing and allows you to manage
/// your own runloop and painting. This method is recommended for those wishing to integrate the
/// library into a game.
///
/// This singleton manages the lifetime of all Views and coordinates all painting, rendering,
/// network requests, and event dispatch.
///
/// You should only call this once per process lifetime.
///
/// You must set up your platform handlers before calling this. At a minimum, you must call
/// ulPlatformSetFileSystem() and  ulPlatformSetFontLoader() before calling this.
///
/// @note  You should not call this if you are using ulCreateApp(), it creates its own renderer and
///        provides default implementations for various platform handlers automatically.
///
ULExport ULRenderer ulCreateRenderer(ULConfig config);

///
/// Destroy the renderer.
///
ULExport void ulDestroyRenderer(ULRenderer renderer);

///
/// Update timers and dispatch internal callbacks (JavaScript and network).
///
ULExport void ulUpdate(ULRenderer renderer);

///
/// Notify the renderer that a display has refreshed (you should call this after vsync).
///
/// This updates animations, smooth scroll, and window.requestAnimationFrame() for all Views
/// matching the display id.
///
ULExport void ulRefreshDisplay(ULRenderer renderer, unsigned int display_id);

///
/// Render all active Views.
///
ULExport void ulRender(ULRenderer renderer);

///
/// Attempt to release as much memory as possible. Don't call this from any callbacks or driver
/// code.
///
ULExport void ulPurgeMemory(ULRenderer renderer);

///
/// Print detailed memory usage statistics to the log. (@see ulPlatformSetLogger)
///
ULExport void ulLogMemoryUsage(ULRenderer renderer);

///
/// Start the remote inspector server.
///
/// While the remote inspector is active, Views that are loaded into this renderer
/// will be able to be remotely inspected from another Ultralight instance either locally
/// (another app on same machine) or remotely (over the network) by navigating a View to:
///
/// \code
///   inspector://<ADDRESS>:<PORT>
/// \endcode
///
/// @return  Returns whether the server started successfully or not.
///
ULExport bool ulStartRemoteInspectorServer(ULRenderer renderer, const char* address,
                                           unsigned short port);

///
/// Describe the details of a gamepad, to be used with ulFireGamepadEvent and related
/// events below. This can be called multiple times with the same index if the details change.
/// 
/// @param  renderer  The active renderer instance.
///
/// @param  index   The unique index (or "connection slot") of the gamepad. For example,
///                 controller #1 would be "1", controller #2 would be "2" and so on.
///
/// @param  id      A string ID representing the device, this will be made available
///                 in JavaScript as gamepad.id
///
/// @param  axis_count  The number of axes on the device.
///
/// @param  button_count  The number of buttons on the device.
///
ULExport void ulSetGamepadDetails(ULRenderer renderer, unsigned int index, ULString id,
                                  unsigned int axis_count, unsigned int button_count);

///
/// Fire a gamepad event (connection / disconnection).
///
/// @note  The gamepad should first be described via ulSetGamepadDetails before calling this
///        function.
///
/// @see <https://developer.mozilla.org/en-US/docs/Web/API/Gamepad>
///
ULExport void ulFireGamepadEvent(ULRenderer renderer, ULGamepadEvent evt);

///
/// Fire a gamepad axis event (to be called when an axis value is changed).
///
/// @note  The gamepad should be connected via a previous call to ulFireGamepadEvent.
///
/// @see <https://developer.mozilla.org/en-US/docs/Web/API/Gamepad/axes>
///
ULExport void ulFireGamepadAxisEvent(ULRenderer renderer, ULGamepadAxisEvent evt);

///
/// Fire a gamepad button event (to be called when a button value is changed).
///
/// @note  The gamepad should be connected via a previous call to ulFireGamepadEvent.
///
/// @see <https://developer.mozilla.org/en-US/docs/Web/API/Gamepad/buttons>
///
ULExport void ulFireGamepadButtonEvent(ULRenderer renderer, ULGamepadButtonEvent evt);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_RENDERER_H