/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_GAMEPADEVENT_H
#define ULTRALIGHT_CAPI_GAMEPADEVENT_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Gamepad Event
 *****************************************************************************/

///
/// Create a gamepad event, see GamepadEvent for help using this function.
///
ULExport ULGamepadEvent ulCreateGamepadEvent(unsigned int index, ULGamepadEventType type);

///
/// Destroy a gamepad event.
///
ULExport void ulDestroyGamepadEvent(ULGamepadEvent evt);

/******************************************************************************
 * Gamepad Axis Event
 *****************************************************************************/

///
/// Create a gamepad axis event, see GamepadAxisEvent for help using this function.
///
ULExport ULGamepadAxisEvent ulCreateGamepadAxisEvent(unsigned int index, unsigned int axis_index,
                                                     double value);

///
/// Destroy a gamepad axis event.
///
ULExport void ulDestroyGamepadAxisEvent(ULGamepadAxisEvent evt);

/******************************************************************************
 * Gamepad Button Event
 *****************************************************************************/

///
/// Create a gamepad button event, see GamepadButtonEvent for help using this function.
///
ULExport ULGamepadButtonEvent ulCreateGamepadButtonEvent(unsigned int index,
                                                         unsigned int button_index, double value);

///
/// Destroy a gamepad button event.
///
ULExport void ulDestroyGamepadButtonEvent(ULGamepadButtonEvent evt);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_GAMEPADEVENT_H