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
/// The winding order for front-facing triangles. (Only used when the GPU renderer is used)
///
enum class FaceWinding : uint8_t {
  ///
  /// Clockwise Winding (Direct3D, etc.)
  ///
  Clockwise,

  ///
  /// Counter-Clockwise Winding (OpenGL, etc.)
  ///
  CounterClockwise,
};

enum class FontHinting : uint8_t {
  ///
  /// Lighter hinting algorithm-- glyphs are slightly fuzzier but better resemble their original
  /// shape. This is achieved by snapping glyphs to the pixel grid only vertically which better
  /// preserves inter-glyph spacing.
  ///
  Smooth,

  ///
  /// Default hinting algorithm-- offers a good balance between sharpness and shape at smaller font
  /// sizes.
  ///
  Normal,

  ///
  /// Strongest hinting algorithm-- outputs only black/white glyphs. The result is usually
  /// unpleasant if the underlying TTF does not contain hints for this type of rendering.
  ///
  Monochrome,

  ///
  /// No hinting is performed-- fonts may be blurry at smaller font sizes.
  ///
  None,
};

enum class EffectQuality : uint8_t {
  ///
  /// Fastest effect quality-- uses the lowest quality effects (half-resolution, fewer passes, etc.)
  /// 
  Low,

  ///
  /// Default effect quality-- strikes a good balance between quality and performance.
  /// 
  Medium,

  ///
  /// Highest effect quality-- favors quality over performance.
  /// 
  High,
};

///
/// Global config for Ultralight.
///
/// This can be used to configure the library by calling Platform::set_config() before creating the
/// Renderer or App singletons.
/// 
/// @par Example usage
/// ```
///   Config config;
///   config.user_stylesheet = "body { background: purple; }";
/// 
///   Platform::instance().set_config(config);
///   // (Setup other Platform interfaces here.)
/// 
///   auto renderer = Renderer::Create();
/// ```
///
struct UExport Config {
  ///
  /// A writable OS file path to store persistent Session data in.
  /// 
  /// This data may include cookies, cached network resources, indexed DB, etc.
  /// 
  /// @note Files are only written to the path when using a persistent Session.
  /// 
  /// @see Renderer::CreateSession()
  ///
  String cache_path;

  ///
  /// The relative path to the resources folder (loaded via the FileSystem API).
  /// 
  /// The library loads certain resources (SSL certs, ICU data, etc.) from the FileSystem API
  /// during runtime (eg, `file:///resources/cacert.pem`).
  /// 
  /// You can customize the relative file path to the resources folder by modifying this setting.
  /// 
  /// @see FileSystem
  ///
  String resource_path_prefix = "resources/";

  ///
  /// The winding order for front-facing triangles.
  ///
  /// @pre Only used when GPU rendering is enabled for the View.
  ///
  /// @see FaceWinding
  ///
  FaceWinding face_winding = FaceWinding::CounterClockwise;

  ///
  /// The hinting algorithm to use when rendering fonts.
  /// 
  /// @see FontHinting
  ///
  FontHinting font_hinting = FontHinting::Normal;

  ///
  /// The gamma to use when compositing font glyphs.
  /// 
  /// You can change this value to adjust font contrast (Adobe and Apple prefer 1.8).
  ///
  double font_gamma = 1.8;

  ///
  /// Global user-defined CSS string (included before any CSS on the page).
  /// 
  /// You can use this to override default styles for various elements on the page.
  /// 
  /// @note This is an actual string of CSS, not a file path.
  ///
  String user_stylesheet;

  ///
  /// Whether or not to continuously repaint any Views, regardless if they are dirty.
  /// 
  /// This is mainly used to diagnose painting/shader issues and profile performance.
  ///
  bool force_repaint = false;

  ///
  /// The delay (in seconds) between every tick of a CSS animation. (Default: 60 FPS)
  ///
  double animation_timer_delay = 1.0 / 60.0;

  ///
  /// The delay (in seconds) between every tick of a smooth scroll animation. (Default: 60 FPS)
  ///
  double scroll_timer_delay = 1.0 / 60.0;

  ///
  /// The delay (in seconds) between every call to the recycler.
  /// 
  /// The library attempts to reclaim excess memory during calls to the internal recycler. You can
  /// change how often this is run by modifying this value.
  ///
  double recycle_delay = 4.0;

  ///
  /// The size of WebCore's memory cache in bytes.
  ///
  /// @note  You should increase this if you anticipate handling pages with large resources, Safari
  ///        typically uses 128+ MiB for its cache.
  ///
  uint32_t memory_cache_size = 64 * 1024 * 1024;

  ///
  /// The number of pages to keep in the cache. (Default: 0, none)
  ///
  /// @note
  /// \parblock
  /// 
  /// Safari typically caches about 5 pages and maintains an on-disk cache to support typical
  /// web-browsing activities.
  /// 
  /// If you increase this, you should probably increase the memory cache size as well.
  /// 
  /// \endparblock
  ///
  uint32_t page_cache_size = 0;

  ///
  /// The system's physical RAM size in bytes.
  /// 
  /// JavaScriptCore tries to detect the system's physical RAM size to set reasonable allocation
  /// limits. Set this to anything other than 0 to override the detected value. Size is in bytes.
  ///
  /// This can be used to force JavaScriptCore to be more conservative with its allocation strategy
  /// (at the cost of some performance).
  ///
  uint32_t override_ram_size = 0;

  ///
  /// The minimum size of large VM heaps in JavaScriptCore.
  /// 
  /// Set this to a lower value to make these heaps start with a smaller initial value.
  ///
  uint32_t min_large_heap_size = 32 * 1024 * 1024;

  ///
  /// The minimum size of small VM heaps in JavaScriptCore.
  /// 
  /// Set this to a lower value to make these heaps start with a smaller initial value.
  ///
  uint32_t min_small_heap_size = 1 * 1024 * 1024;

  ///
  /// The number of threads to use in the Renderer (for parallel painting on the CPU, etc.).
  /// 
  /// You can set this to a certain number to limit the number of threads to spawn.
  /// 
  /// @note
  /// \parblock
  /// 
  /// If this value is 0, the number of threads will be determined at runtime using the following
  /// formula: 
  /// 
  ///   ```
  ///   max(PhysicalProcessorCount() - 1, 1)
  ///   ```
  /// 
  /// \endparblock
  /// 
  uint32_t num_renderer_threads = 0;

  /// 
  /// The max amount of time (in seconds) to allow repeating timers to run during each call to
  /// Renderer::Update.
  /// 
  /// The library will attempt to throttle timers if this time budget is exceeded.
  ///
  double max_update_time = 1.0 / 200.0;

  ///
  /// The alignment (in bytes) of the BitmapSurface when using the CPU renderer.
  ///
  /// The underlying bitmap associated with each BitmapSurface will have row_bytes padded to reach
  /// this alignment.
  ///
  /// Aligning the bitmap helps improve performance when using the CPU renderer. Determining the
  /// proper value to use depends on the CPU architecture and max SIMD instruction set used.
  ///
  /// We generally target the 128-bit SSE2 instruction set across most PC platforms so '16' is
  /// a safe value to use.
  ///
  /// You can set this to '0' to perform no padding (row_bytes will always be width * 4) at a
  /// slight cost to performance.
  ///
  uint32_t bitmap_alignment = 16;

  ///
  /// The quality of effects (blurs, CSS filters, SVG filters, etc.) to use when rendering.
  /// 
  EffectQuality effect_quality = EffectQuality::Medium;
};

} // namespace ultralight
