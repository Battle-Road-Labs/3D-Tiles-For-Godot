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
#include <Ultralight/Renderer.h>
#include <Ultralight/platform/Config.h>

namespace ultralight {

class Monitor;
class Window;

///
/// Interface for all App-related events. @see App::set_listener
///
class AExport AppListener {
public:
  virtual ~AppListener() {}

  ///
  /// Called whenever the App updates. You should update all app logic here.
  ///
  /// @note  This event is fired right before the run loop calls
  ///        Renderer::Update and Renderer::Render.
  ///
  virtual void OnUpdate() {}
};

///
/// App-specific settings.
///
struct AExport Settings {
  ///
  /// The name of the developer of this app.
  ///
  /// This is used to generate a unique path to store local application data
  /// on the user's machine.
  ///
  String developer_name = "MyCompany";

  ///
  /// The name of this app.
  ///
  /// This is used to generate a unique path to store local application data
  /// on the user's machine.
  ///
  String app_name = "MyApp";

  ///
  /// The root file path for our file system. You should set this to the
  /// relative path where all of your app data is.
  ///
  /// This will be used to resolve all file URLs, eg file:///page.html
  ///
  /// @note  This relative path is resolved using the following logic:
  ///         - Windows: relative to the executable path
  ///         - Linux:   relative to the executable path
  ///         - macOS:   relative to YourApp.app/Contents/Resources/
  ///
  String file_system_path = "./assets/";

  ///
  /// Whether or not we should load and compile shaders from the file system
  /// (eg, from the /shaders/ path, relative to file_system_path).
  ///
  /// If this is false (the default), we will instead load pre-compiled shaders
  /// from memory which speeds up application startup time.
  ///
  bool load_shaders_from_file_system = false;

  ///
  /// We try to use the GPU renderer when a compatible GPU is detected.
  ///
  /// Set this to true to force the engine to always use the CPU renderer.
  ///
  bool force_cpu_renderer = false;
};

///
/// Main application singleton (use this if you want to let the library manage window creation).
/// 
/// This convenience class automatically sets up the Renderer, creates a run loop, and handles all
/// painting and platform-specific operations for you.
///
class AExport App : public RefCounted {
public:
  ///
  /// Create the App singleton.
  ///
  /// @param  settings  Settings to customize App runtime behavior.
  ///
  /// @param  config  Config options for the Ultralight renderer.
  ///
  /// @return  Returns a ref-pointer to the created App instance.
  ///
  /// @note  You should only create one of these per application lifetime.
  ///
  /// @note  Certain Config options may be overridden during App creation,
  ///        most commonly Config::face_winding and Config::cache_path.
  ///
  static RefPtr<App> Create(Settings settings = Settings(), Config config = Config());

  ///
  /// Get the App singleton.
  ///
  static App* instance();

  ///
  /// Get the settings this App was created with.
  ///
  virtual const Settings& settings() const = 0;

  ///
  /// Set an AppListener to receive callbacks for app-related events.
  ///
  /// @note  Ownership remains with the caller.
  ///
  virtual void set_listener(AppListener* listener) = 0;

  ///
  /// Get the AppListener, if any.
  ///
  virtual AppListener* listener() = 0;

  ///
  /// Whether or not the App is running.
  ///
  virtual bool is_running() const = 0;

  ///
  /// Get the main monitor (this is never NULL).
  ///
  /// @note  We'll add monitor enumeration later.
  ///
  virtual Monitor* main_monitor() = 0;

  ///
  /// Get the underlying Renderer instance.
  ///
  virtual RefPtr<Renderer> renderer() = 0;

  ///
  /// Run the main loop.
  ///
  virtual void Run() = 0;

  ///
  /// Quit the application.
  ///
  virtual void Quit() = 0;

protected:
  virtual ~App();
};

}  // namespace ultralight
