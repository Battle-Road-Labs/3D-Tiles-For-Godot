/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include "Defines.h"
#include <Ultralight/RefPtr.h>
#include <Ultralight/Listener.h>
#include <Ultralight/Bitmap.h>
#include <Ultralight/KeyEvent.h>
#include <Ultralight/MouseEvent.h>
#include <Ultralight/ScrollEvent.h>

namespace ultralight {

class Monitor;
class OverlayManager;
class Surface;
class Window;

///
/// Interface for all Window-related events. @see Window::set_listener
///
class WindowListener {
public:
  virtual ~WindowListener() {}

  ///
  /// Called when the Window is closed.
  ///
  virtual void OnClose(ultralight::Window* window) { }

  ///
  /// Called when the Window is resized.
  ///
  /// @param  width   The new width (in pixels).
  ///
  /// @param  height  The new height (in pixels).
  ///
  virtual void OnResize(ultralight::Window* window, uint32_t width_px, uint32_t height_px) { }

  ///
  /// Called when a keyboard event is fired.
  /// 
  /// @param  evt  Details for the event.
  /// 
  /// @return  Return false to consume the event and prevent it from propagating further.
  /// 
  virtual bool OnKeyEvent(const ultralight::KeyEvent& evt) { return true; }

  ///
  /// Called when a mouse event is fired.
  ///
  /// @param  evt  Details for the event.
  ///
  /// @return  Return false to consume the event and prevent it from propagating further.
  /// 
  virtual bool OnMouseEvent(const ultralight::MouseEvent& evt) { return true; }

  ///
  /// Called when a scroll event is fired.
  ///
  /// @param  evt  Details for the event.
  ///
  /// @return  Return false to consume the event and prevent it from propagating further.
  /// 
  virtual bool OnScrollEvent(const ultralight::ScrollEvent& evt) { return true; }
};

///
/// Window creation flags. @see Window::Create
///
enum WindowFlags : uint8_t {
  kWindowFlags_Borderless  = 1 << 0,
  kWindowFlags_Titled      = 1 << 1,
  kWindowFlags_Resizable   = 1 << 2,
  kWindowFlags_Maximizable = 1 << 3,
  kWindowFlags_Hidden      = 1 << 4,
};

///
/// Window class, represents a platform window.
///
class AExport Window : public RefCounted {
public:
    ///
    /// Create a new Window.
    ///
    /// @param  monitor       The monitor to create the Window on.
    ///
    /// @param  width         The width (in screen coordinates).
    ///
    /// @param  height        The height (in screen coordinates).
    ///
    /// @param  fullscreen    Whether or not the window is fullscreen.
    ///
    /// @param  window_flags  Various window flags.
    ///
    /// @note
    /// \parblock
    ///
    /// Windows are immediately shown by default unless kWindowFlags_Hidden is set in the
    /// window_flags parameter. (They can be shown later via Window::Show())
    ///
    /// \endparblock
    ///
    /// @note
    /// \parblock
    ///
    /// Screen coordinates are device-scale-independent and have the following relationship
    /// to pixel coordinates:
    ///
    /// \code
    /// pixel_coordinate = round(screen_coordinate * scale)
    /// \endcode
    ///
    /// \endparblock
    ///
    static RefPtr<Window> Create(Monitor* monitor, uint32_t width, uint32_t height,
        bool fullscreen, unsigned int window_flags);

    ///
    /// Set a WindowListener to receive callbacks for window-related events.
    ///
    /// @note  Ownership remains with the caller.
    ///
    virtual void set_listener(WindowListener* listener) = 0;

    ///
    /// Get the WindowListener, if any.
    ///
    virtual WindowListener* listener() = 0;

    ///
    /// Get the window width (in screen coordinates).
    ///
    virtual uint32_t screen_width() const = 0;

    ///
    /// Get the window width (in pixels).
    ///
    virtual uint32_t width() const = 0;

    ///
    /// Get the window height (in screen coordinates).
    ///
    virtual uint32_t screen_height() const = 0;

    ///
    /// Get the window height (in pixels).
    ///
    virtual uint32_t height() const = 0;

    ///
    /// Move the window to a new position (in screen coordinates) relative to the top-left of the
    /// monitor area.
    ///
    virtual void MoveTo(int x, int y) = 0;

    ///
    /// Move the window to the center of the monitor.
    ///
    virtual void MoveToCenter() = 0;

    ///
    /// Get the x-position of the window (in screen coordinates) relative to the top-left of the
    /// monitor area.
    ///
    virtual int x() const = 0;

    ///
    /// Get the y-position of the window (in screen coordinates) relative to the top-left of the
    /// monitor area.
    ///
    virtual int y() const = 0;

    ///
    /// Whether or not the window is fullscreen.
    ///
    virtual bool is_fullscreen() const = 0;

    ///
    /// Whether or not the window is GPU accelerated.
    ///
    virtual bool is_accelerated() const = 0;

    ///
    /// The render buffer id of the the window's backing texture.
    /// (This will be 0 if the window is not accelerated).
    ///
    virtual uint32_t render_buffer_id() const = 0;

    ///
    /// The DPI scale of the window.
    ///
    virtual double scale() const = 0;

    ///
    /// Set the window title.
    ///
    virtual void SetTitle(const char* title) = 0;

    ///
    /// Set the cursor.
    ///
    virtual void SetCursor(ultralight::Cursor cursor) = 0;

    ///
    /// Show the window (if it was previously hidden).
    ///
    virtual void Show() = 0;

    ///
    /// Hide the window.
    ///
    virtual void Hide() = 0;

    ///
    /// Whether or not the window is currently visible (not hidden).
    ///
    virtual bool is_visible() const = 0;

    ///
    /// Close the window.
    ///
    virtual void Close() = 0;

    ///
    /// Convert screen coordinates to pixels using the current DPI scale.
    ///
    virtual int ScreenToPixels(int val) const = 0;

    ///
    /// Convert pixels to screen coordinates using the current DPI scale.
    ///
    virtual int PixelsToScreen(int val) const = 0;

    ///
    /// Draw a surface directly to window, used only by CPU renderer
    ///
    virtual void DrawSurface(int x, int y, Surface* surface) {}

    ///
    /// Get the underlying native window handle.
    ///
    /// @note
    ///   This is:
    ///   - HWND on Windows
    ///   - NSWindow* on macOS
    ///   - GLFWwindow* on Linux
    ///
    virtual void* native_handle() const = 0;

    ///
    /// Display frame statistics (FPS, frame times, etc.) in the titlebar.
    /// 
    virtual void EnableFrameStatistics() {}

protected:
  virtual ~Window();
  virtual bool platform_always_uses_cpu_renderer() const = 0;
  virtual OverlayManager* overlay_manager() const = 0;

  friend class OverlayImpl;
};

}  // namespace ultralight
