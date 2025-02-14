/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include "Window.h"
#include <Ultralight/View.h>

namespace ultralight {

///
/// Web-content overlay. Displays a web-page within an area of a Window.
///
/// @note  Each Overlay is essentially a View and an on-screen quad. You should
///        create the Overlay then load content into the underlying View.
///
class AExport Overlay : public RefCounted {
public:
  ///
  /// Create a new Overlay.
  ///
  /// @param  window  The window to create the Overlay in.
  ///
  /// @param  width   The width in pixels.
  ///
  /// @param  height  The height in pixels.
  ///
  /// @param  x       The x-position (offset from the left of the Window), in
  ///                 pixels.
  ///
  /// @param  y       The y-position (offset from the top of the Window), in
  ///                 pixels.
  ///
  static RefPtr<Overlay> Create(RefPtr<Window> window, uint32_t width,
                             uint32_t height, int x, int y);

  ///
  /// Create a new Overlay, wrapping an existing View.
  ///
  /// @param  window  The window to create the Overlay in.
  ///
  /// @param  view    The View to wrap (will use its width and height).
  ///
  /// @param  x       The x-position (offset from the left of the Window), in
  ///                 pixels.
  ///
  /// @param  y       The y-position (offset from the top of the Window), in
  ///                 pixels.
  ///
  static RefPtr<Overlay> Create(RefPtr<Window> window, RefPtr<View> view, int x, int y);

  ///
  /// Get the underlying View.
  ///
  virtual ultralight::RefPtr<ultralight::View> view() = 0;

  ///
  /// Get the width (in pixels).
  ///
  virtual uint32_t width() const = 0;

  ///
  /// Get the height (in pixels).
  ///
  virtual uint32_t height() const = 0;
  
  ///
  /// Get the x-position (offset from the left of the Window), in pixels.
  ///
  virtual int x() const = 0;

  ///
  /// Get the y-position (offset from the top of the Window), in pixels.
  ///
  virtual int y() const = 0;

  ///
  /// Whether or not the overlay is hidden (not drawn).
  ///
  virtual bool is_hidden() const = 0;

  ///
  /// Hide the overlay (will no longer be drawn)
  ///
  virtual void Hide() = 0;

  ///
  /// Show the overlay.
  ///
  virtual void Show() = 0;

  ///
  /// Whether or not this overlay has keyboard focus.
  ///
  virtual bool has_focus() const = 0;

  ///
  /// Grant this overlay exclusive keyboard focus.
  ///
  virtual void Focus() = 0;

  ///
  /// Remove keyboard focus.
  ///
  virtual void Unfocus() = 0;

  ///
  /// Move the overlay to a new position (in pixels).
  ///
  virtual void MoveTo(int x, int y) = 0;

  ///
  /// Resize the overlay (and underlying View), dimensions should be
  /// specified in pixels.
  ///
  virtual void Resize(uint32_t width, uint32_t height) = 0;

  ///
  /// Whether or not this Overlay needs repaint (either it has moved, resized,
  /// or the internal View needs repaint).
  ///
  virtual bool NeedsRepaint() = 0;

protected:
  virtual ~Overlay();
  virtual void Render() = 0;
  virtual void Paint() = 0;
  friend class OverlayManager;
};

}  // namespace framework
