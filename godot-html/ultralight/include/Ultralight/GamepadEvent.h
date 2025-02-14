/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/String.h>

namespace ultralight {

///
/// Event representing a change in gamepad connection state.
///
/// @see  Renderer::FireGamepadEvent
///
class UExport GamepadEvent {
 public:
  ///
  /// The various GamepadEvent types.
  ///
  enum Type {
    ///
    /// This event type should be fired when a gamepad is connected.
    ///
    /// @note  You will need to previously declare the gamepad, its index, and details about
    ///        its axis and button layout via Renderer::SetGamepadDetails prior to calling
    ///        Renderer::FireGamepadEvent.
    /// 
    kType_GamepadConnected,
    
    ///
    /// This event type should be fired when a gamepad is disconnected.
    ///
    kType_GamepadDisconnected,
  };

  ///
  // The type of this GamepadEvent
  ///
  Type type;

  ///
  /// The index of the gamepad, this should match the value previously set in
  /// Renderer::SetGamepadDetails.
  ///
  uint32_t index;
};

///
/// Event representing a change in gamepad axis state (eg, pressing a stick in a certain direction).
///
/// @see  Renderer::FireGamepadAxisEvent
///
class UExport GamepadAxisEvent {
 public:
  ///
  /// The index of the gamepad, this should match the value previously set in
  /// Renderer::SetGamepadDetails.
  ///
  uint32_t index;

  ///
  /// The index of the axis whose value has changed.
  ///
  /// This value should be in the range previously set in Renderer::SetGamepadDetails.
  ///
  uint32_t axis_index;

  ///
  /// The new value of the axis.
  ///
  /// This value should be normalized to the range [-1.0, 1.0].
  ///
  double value;
};

///
/// Event representing a change in gamepad button state (eg, pressing a button on a gamepad).
///
/// @see    Renderer::FireGamepadButtonEvent
///
class UExport GamepadButtonEvent {
 public:
  ///
  /// The index of the gamepad, this should match the value previously set in
  /// Renderer::SetGamepadDetails.
  ///
  uint32_t index;

  ///
  /// The index of the button whose value has changed.
  ///
  /// This value should be in the range previously set in Renderer::SetGamepadDetails.
  ///
  uint32_t button_index;

  ///
  /// The new value of the button.
  ///
  /// This value should be normalized to the range [-1.0, 1.0], with any value greater than
  /// 0.0 to be considered "pressed".
  ///
  double value;
};

}  // namespace ultralight
