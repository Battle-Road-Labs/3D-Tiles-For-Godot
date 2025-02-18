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

struct Config;
class Logger;
class GPUDriver;
class FontLoader;
class FileSystem;
class Clipboard;
class SurfaceFactory;
class ThreadFactory;

///
/// Global platform singleton, manages user-defined platform handlers and global config.
/// 
/// The library uses the Platform API for most platform-specific operations (eg, file access,
/// clipboard, font loading, GPU access, etc.).
/// 
/// @par Overview of which platform handlers are required / optional / provided:
/// 
/// |                | Renderer::Create() | App::Create() |
/// |----------------|--------------------|---------------|
/// | FileSystem     | **Required**       | *Provided*    |
/// | FontLoader     | **Required**       | *Provided*    |
/// | Clipboard      |  *Optional*        | *Provided*    |
/// | GPUDriver      |  *Optional*        | *Provided*    |
/// | Logger         |  *Optional*        | *Provided*    |
/// | SurfaceFactory |  *Provided*        | *Provided*    |
/// 
/// @note  This singleton should be set up before creating the Renderer or App.
///
class UExport Platform {
 public:
  ///
  /// Get the Platform singleton
  ///
  static Platform& instance();

  virtual ~Platform();

  ///
  /// Set the Config
  ///
  virtual void set_config(const Config& config) = 0;

  ///
  /// Get the Config
  ///
  virtual const Config& config() const = 0;

  ///
  /// Set the Logger (to handle error messages and debug output).
  ///
  /// @param  logger  A user-defined Logger implementation, ownership remains with the caller.
  ///
  virtual void set_logger(Logger* logger) = 0;

  ///
  /// Get the Logger
  ///
  virtual Logger* logger() const = 0;

  ///
  /// Set the GPU Driver (will handle all rendering)
  ///
  /// @param  gpu_driver  A user-defined GPUDriver implementation, ownership remains with the
  ///                     caller.
  ///
  virtual void set_gpu_driver(GPUDriver* gpu_driver) = 0;

  ///
  /// Get the GPU Driver
  ///
  virtual GPUDriver* gpu_driver() const = 0;

  ///
  /// Set the Font Loader (will be used to map font families to actual fonts)
  ///
  /// @param  font_loader  A user-defined FontLoader implementation, ownership remains with the
  ///                      caller.
  ///
  virtual void set_font_loader(FontLoader* font_loader) = 0;

  ///
  /// Get the Font Loader
  ///
  virtual FontLoader* font_loader() const = 0;

  ///
  /// Set the File System (will be used for all file system operations)
  ///
  /// @param  file_system  A user-defined FileSystem implementation, ownership remains with the
  ///                      caller.
  ///
  virtual void set_file_system(FileSystem* file_system) = 0;

  ///
  /// Get the File System
  ///
  virtual FileSystem* file_system() const = 0;

  ///
  /// Set the Clipboard (will be used for all clipboard operations)
  ///
  /// @param  clipboard  A user-defined Clipboard implementation, ownership remains with the
  ///                    caller.
  ///
  virtual void set_clipboard(Clipboard* clipboard) = 0;

  ///
  /// Get the Clipboard
  ///
  virtual Clipboard* clipboard() const = 0;

  ///
  /// Set the SurfaceFactory
  ///
  /// This can be used to provide a platform-specific bitmap surface for View to paint into when
  /// the CPU renderer is enabled. See View::surface().
  ///
  /// @param  surface_factory  A user-defined SurfaceFactory implementation, ownership remains with
  ///                          the caller.
  ///
  /// @note  A default BitmapSurfaceFactory is defined if you never call this, View::surface() can
  ///        be safely cast to BitmapSurface.
  ///
  virtual void set_surface_factory(SurfaceFactory* surface_factory) = 0;

  ///
  /// Get the SurfaceFactory
  ///
  /// @note  A default BitmapSurfaceFactory is set by default, View::surface() can be safely cast
  ///        to BitmapSurface if you don't define your own.
  ///
  virtual SurfaceFactory* surface_factory() const = 0;

  virtual void set_thread_factory(ThreadFactory* thread_factory) = 0;
 
  virtual ThreadFactory* thread_factory() const = 0;
};

} // namespace ultralight
