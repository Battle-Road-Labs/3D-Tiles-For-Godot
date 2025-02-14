/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <Ultralight/RefPtr.h>
#include <Ultralight/KeyEvent.h>
#include <Ultralight/JavaScript.h>
#include <Ultralight/MouseEvent.h>
#include <Ultralight/ScrollEvent.h>
#include <Ultralight/GamepadEvent.h>
#include <Ultralight/RenderTarget.h>
#include <Ultralight/Bitmap.h>
#include <Ultralight/Listener.h>
#include <Ultralight/platform/Surface.h>

namespace ultralight {

///
/// View-specific configuration settings.
/// 
/// @see Renderer::CreateView
/// 
struct UExport ViewConfig {
  
  ///
  /// A user-generated id for the display (monitor, TV, or screen) that this View will be shown on.
  /// 
  /// Animations are driven based on the physical refresh rate of the display. Multiple Views can
  /// share the same display.
  /// 
  /// @note This is automatically managed for you when App::Create() is used.
  /// 
  /// @see Renderer::RefreshDisplay.
  /// 
  uint32_t display_id = 0;

  ///
  /// Whether to render using the GPU renderer (accelerated) or the CPU renderer (unaccelerated).
  ///
  /// When true, the View will be rendered to an offscreen GPU texture using the GPU driver set in
  /// Platform::set_gpu_driver. You can fetch details for the texture via View::render_target.
  ///
  /// When false (the default), the View will be rendered to an offscreen pixel buffer using the
  /// multithreaded CPU renderer. This pixel buffer can optionally be provided by the user--
  /// for more info see Platform::set_surface_factory and View::surface.
  /// 
  /// @note This is automatically managed for you when App::Create() is used.
  ///
  bool is_accelerated = false;

  ///
  /// The initial device scale, ie. the amount to scale page units to screen pixels. This should
  /// be set to the scaling factor of the device that the View is displayed on.
  ///
  /// @note 1.0 is equal to 100% zoom (no scaling), 2.0 is equal to 200% zoom (2x scaling)
  ///
  /// @note This is automatically managed for you when App::Create() is used.
  ///
  double initial_device_scale = 1.0;

  ///
  /// Whether or not this View should support transparency.
  ///
  /// @note Make sure to also set the following CSS on the page:
  ///
  ///    html, body { background: transparent; }
  ///
  bool is_transparent = false;

  ///
  /// Whether or not the View should initially have input focus, @see View::Focus()
  ///
  bool initial_focus = true;

  ///
  /// Whether or not images should be enabled.
  ///
  bool enable_images = true;

  ///
  /// Whether or not JavaScript should be enabled.
  ///
  bool enable_javascript = true;
  
  ///
  /// Whether or not compositing should be enabled.
  /// 
  bool enable_compositor = false;

  ///
  /// Default font-family to use.
  ///
  String font_family_standard = "Times New Roman";

  ///
  /// Default font-family to use for fixed fonts. (pre/code)
  ///
  String font_family_fixed = "Courier New";

  ///
  /// Default font-family to use for serif fonts.
  ///
  String font_family_serif = "Times New Roman";

  ///
  /// Default font-family to use for sans-serif fonts.
  ///
  String font_family_sans_serif = "Arial";

  ///
  /// Default user-agent string.
  ///
  String user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                      "AppleWebKit/615.1.18.100.1 (KHTML, like Gecko) "
                      "Ultralight/1.4.0 Version/16.4.1 Safari/615.1.18.100.1";
};

///
/// Web-page container rendered to an offscreen surface that you display yourself.
/// 
/// The View class is responsible for loading and rendering web-pages to an offscreen surface. It
/// is completely isolated from the OS windowing system, you must forward all input events to it
/// from your application.
///
/// @note  The API is not thread-safe, all calls must be made on the same thread that the
///        Renderer/App was created on.
///
class UExport View : public RefCounted {
 public:
  ///
  /// Get the URL of the current page loaded into this View, if any.
  ///
  virtual String url() = 0;

  ///
  /// Get the title of the current page loaded into this View, if any.
  ///
  virtual String title() = 0;

  ///
  /// Get the width of the View, in pixels.
  ///
  virtual uint32_t width() const = 0;

  ///
  /// Get the height of the View, in pixels.
  ///
  virtual uint32_t height() const = 0;

  ///
  /// Get the display id of the View.
  /// 
  /// @see ViewConfig::display_id
  /// 
  virtual uint32_t display_id() const = 0;

  ///
  /// Set the display id of the View.
  /// 
  /// This should be called when the View is moved to another display.
  /// 
  virtual void set_display_id(uint32_t id) = 0;

  ///
  /// Get the device scale, ie. the amount to scale page units to screen pixels.
  /// 
  /// For example, a value of 1.0 is equivalent to 100% zoom. A value of 2.0 is 200% zoom.
  /// 
  virtual double device_scale() const = 0;

  ///
  /// Set the device scale.
  /// 
  virtual void set_device_scale(double scale) = 0;

  ///
  /// Whether or not the View is GPU-accelerated. If this is false, the page will be rendered
  /// via the CPU renderer.
  /// 
  virtual bool is_accelerated() const = 0;

  ///
  /// Whether or not the View supports transparent backgrounds.
  ///
  virtual bool is_transparent() const = 0;

  ///
  /// Check if the main frame of the page is currently loading.
  ///
  virtual bool is_loading() = 0;

  ///
  /// Get the RenderTarget for the View.
  ///
  /// @pre  Only valid if this View is using the GPU renderer (see ViewConfig::is_accelerated).
  ///
  /// @note  You can use this with your GPUDriver implementation to bind and display the
  ///        corresponding texture in your application.
  ///
  virtual RenderTarget render_target() = 0;

  ///
  /// Get the Surface for the View (native pixel buffer that the CPU renderer draws into).
  ///
  /// @pre  This operation is only valid if the View is using the CPU renderer, (eg, it is
  ///       **not** GPU accelerated, see ViewConfig::is_accelerated). This function will return
  ///       return nullptr if the View is using the GPU renderer.
  /// 
  /// @note  The default Surface is BitmapSurface but you can provide your own Surface
  ///        implementation via Platform::set_surface_factory().
  ///
  virtual Surface* surface() = 0;

  ///
  /// Load a raw string of HTML, the View will navigate to it as a new page.
  ///
  /// @param  html  The raw HTML string to load.
  ///
  /// @param  url   An optional URL for this load (to make it appear as if we we loaded this HTML
  ///               from a certain URL). Can be used for resolving relative URLs and cross-origin
  ///               rules.
  ///
  /// @param  add_to_history  Whether or not this load should be added to the session's history
  ///                         (eg, the back/forward list).
  ///
  virtual void LoadHTML(const String& html, const String& url = "", bool add_to_history = false)
      = 0;

  ///
  /// Load a URL, the View will navigate to it as a new page.
  ///
  /// @note  You can use File URLs (eg, file:///page.html) but you must define your own FileSystem
  ///        implementation if you are not using AppCore. @see Platform::set_file_system
  ///
  virtual void LoadURL(const String& url) = 0;

  ///
  /// Resize View to a certain size.
  ///
  /// @param  width   The initial width, in pixels.
  ///
  /// @param  height  The initial height, in pixels.
  ///
  ///
  virtual void Resize(uint32_t width, uint32_t height) = 0;

  ///
  /// Acquire the page's JSContext for use with the JavaScriptCore API
  ///
  /// @note  You can use the underlying JSContextRef with the JavaScriptCore C API. This allows you
  ///        to marshall C/C++ objects to/from JavaScript, bind callbacks, and call JS functions
  ///        directly.
  ///
  /// @note  The JSContextRef gets reset after each page navigation. You should initialize your
  ///        JavaScript state within the OnWindowObjectReady and OnDOMReady events,
  ///        @see ViewListener.
  ///
  /// @note  This call locks the internal context for the current thread. It will be unlocked when
  ///        the returned JSContext's ref-count goes to zero. The lock is recursive, you can call
  ///        this multiple times.
  ///
  virtual RefPtr<JSContext> LockJSContext() = 0;

  ///
  /// Get a handle to the internal JavaScriptCore VM.
  ///
  virtual void* JavaScriptVM() = 0;

  ///
  /// Helper function to evaluate a raw string of JavaScript and return the result as a String.
  ///
  /// @param  script     A string of JavaScript to evaluate in the main frame.
  ///
  /// @param  exception  A string to store the exception in, if any. Pass a nullptr if you don't
  ///                    care about exceptions.
  ///
  /// @return  Returns the JavaScript result typecast to a String.
  ///
  ///
  /// @note  You do not need to lock the JS context, it is done automatically.
  ///
  /// @note  If you need lower-level access to native JavaScript values, you should instead lock
  ///        the JS context and call JSEvaluateScript() in the JavaScriptCore C API.
  ///        @see <JavaScriptCore/JSBase.h>
  ///
  virtual String EvaluateScript(const String& script, String* exception = nullptr) = 0;

  ///
  /// Whether or not we can navigate backwards in history
  ///
  virtual bool CanGoBack() = 0;

  ///
  /// Whether or not we can navigate forwards in history
  ///
  virtual bool CanGoForward() = 0;

  ///
  /// Navigate backwards in history
  ///
  virtual void GoBack() = 0;

  ///
  /// Navigate forwards in history
  ///
  virtual void GoForward() = 0;

  ///
  /// Navigate to an arbitrary offset in history
  ///
  virtual void GoToHistoryOffset(int offset) = 0;

  ///
  /// Reload current page
  ///
  virtual void Reload() = 0;

  ///
  /// Stop all page loads
  ///
  virtual void Stop() = 0;

  ///
  /// Give focus to the View.
  ///
  /// You should call this to give visual indication that the View has input focus (changes active
  /// text selection colors, for example).
  ///
  virtual void Focus() = 0;

  ///
  /// Remove focus from the View and unfocus any focused input elements.
  ///
  /// You should call this to give visual indication that the View has lost input focus.
  ///
  virtual void Unfocus() = 0;

  ///
  /// Whether or not the View has focus.
  ///
  virtual bool HasFocus() = 0;

  ///
  /// Whether or not the View has an input element with visible keyboard focus (indicated by a
  /// blinking caret).
  ///
  /// You can use this to decide whether or not the View should consume keyboard input events
  /// (useful in games with mixed UI and key handling).
  ///
  virtual bool HasInputFocus() = 0;

  ///
  /// Fire a keyboard event
  ///
  /// @note  Only 'Char' events actually generate text in input fields.
  ///
  virtual void FireKeyEvent(const KeyEvent& evt) = 0;

  ///
  /// Fire a mouse event
  ///
  virtual void FireMouseEvent(const MouseEvent& evt) = 0;

  ///
  /// Fire a scroll event
  ///
  virtual void FireScrollEvent(const ScrollEvent& evt) = 0;

  ///
  /// Set a ViewListener to receive callbacks for View-related events.
  ///
  /// @note  Ownership remains with the caller.
  ///
  virtual void set_view_listener(ViewListener* listener) = 0;

  ///
  /// Get the active ViewListener, if any
  ///
  virtual ViewListener* view_listener() const = 0;

  ///
  /// Set a LoadListener to receive callbacks for Load-related events.
  ///
  /// @note  Ownership remains with the caller.
  ///
  virtual void set_load_listener(LoadListener* listener) = 0;

  ///
  /// Get the active LoadListener, if any
  ///
  virtual LoadListener* load_listener() const = 0;

  ///
  /// Set a DownloadListener to receive callbacks for download-related events.
  ///
  /// @note  Ownership remains with the caller.
  ///
  virtual void set_download_listener(DownloadListener* listener) = 0;

  ///
  /// Get the active DownloadListener, if any
  ///
  virtual DownloadListener* download_listener() const = 0;

  ///
  /// Cancel an active download.
  /// 
  virtual void CancelDownload(DownloadId id) = 0;

  ///
  /// Set a NetworkListener to receive callbacks for network-related events.
  ///
  /// @note  Ownership remains with the caller.
  ///
  virtual void set_network_listener(NetworkListener* listener) = 0;

  ///
  /// Get the active NetworkListener, if any
  ///
  virtual NetworkListener* network_listener() const = 0;

  ///
  /// Set whether or not this View should be repainted during the next call to Renderer::Render
  ///
  /// @note  This flag is automatically set whenever the page content changes but you can set it
  ///        directly in case you need to force a repaint.
  ///
  virtual void set_needs_paint(bool needs_paint) = 0;

  ///
  /// Whether or not this View should be repainted during the next call to
  /// Renderer::Render.
  ///
  virtual bool needs_paint() const = 0;

  ///
  /// Create an Inspector View to inspect / debug this View locally.
  /// 
  /// This will only succeed if you have the inspector assets in your filesystem-- the inspector
  /// will look for file:///inspector/Main.html when it first loads.
  /// 
  /// You must handle ViewListener::OnCreateInspectorView so that the library has a View to display
  /// the inspector in. This function will call this event only if an inspector view is not
  /// currently active.
  ///
  virtual void CreateLocalInspectorView() = 0;

 protected:
  virtual ~View();
};

} // namespace ultralight
