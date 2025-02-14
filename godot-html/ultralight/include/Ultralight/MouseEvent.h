/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>

namespace ultralight {

///
/// Generic mouse event representing a change in mouse state.
///
/// @see View::FireMouseEvent
///
class MouseEvent {
public:
  ///
  /// The various MouseEvent types.
  ///
  enum Type {
	///
    /// Mouse moved type
	///
    kType_MouseMoved,

	///
    /// Mouse button pressed type
	///
    kType_MouseDown,

	///
    /// Mouse button released type
	///
    kType_MouseUp,
  };

  ///
  /// The various mouse button types.
  ///
  enum Button {
    kButton_None = 0,
    kButton_Left,
    kButton_Middle,
    kButton_Right,
  };

  ///
  /// The type of this MouseEvent
  ///
  Type type;

  ///
  /// The current x-position of the mouse, relative to the View
  ///
  int x;

  ///
  /// The current y-position of the mouse, relative to the View
  ///
  int y;

  ///
  /// The mouse button that was pressed/released, if any.
  ///
  Button button;
};

}  // namespace ultralight
