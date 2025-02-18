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
/// Generic scroll event representing a change in scroll state.
///
/// @see View::FireScrollEvent
///
class ScrollEvent {
 public:
  ///
  /// The scroll event granularity type
  ///
  enum Type {
    /// The delta value is interpreted as number of pixels
    kType_ScrollByPixel,

    /// The delta value is interpreted as number of pages
    kType_ScrollByPage,
  };

  ///
  /// Scroll granularity type
  ///
  Type type;

  ///
  /// Horizontal scroll amount
  ///
  int delta_x;

  ///
  /// Vertical scroll amount
  ///
  int delta_y;
};

} // namespace ultralight
