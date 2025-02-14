/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/

///
/// @file CAPI_Config.h
///
/// Global config for Ultralight.
///
/// This can be used to configure the library when calling ulCreateRenderer() or ulCreateApp().
/// 
#ifndef ULTRALIGHT_CAPI_CONFIG_H
#define ULTRALIGHT_CAPI_CONFIG_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Config
 *****************************************************************************/

///
/// Create config with default values (see <Ultralight/platform/Config.h>).
///
ULExport ULConfig ulCreateConfig();

///
/// Destroy config.
///
ULExport void ulDestroyConfig(ULConfig config);

///
/// A writable OS file path to store persistent Session data in.
///
/// This data may include cookies, cached network resources, indexed DB, etc.
///
/// @note Files are only written to the path when using a persistent Session.
///
ULExport void ulConfigSetCachePath(ULConfig config, ULString cache_path);

///
/// The relative path to the resources folder (loaded via the FileSystem API).
///
/// The library loads certain resources (SSL certs, ICU data, etc.) from the FileSystem API
/// during runtime (eg, `file:///resources/cacert.pem`).
///
/// You can customize the relative file path to the resources folder by modifying this setting.
///
/// (Default = "resources/")
///
ULExport void ulConfigSetResourcePathPrefix(ULConfig config, ULString resource_path_prefix);

///
/// The winding order for front-facing triangles.
///
/// @pre Only used when GPU rendering is enabled for the View.
///
/// (Default = kFaceWinding_CounterClockwise)
///
ULExport void ulConfigSetFaceWinding(ULConfig config, ULFaceWinding winding);

///
/// The hinting algorithm to use when rendering fonts. (Default = kFontHinting_Normal)
/// 
/// @see ULFontHinting
///
ULExport void ulConfigSetFontHinting(ULConfig config, ULFontHinting font_hinting);

///
/// The gamma to use when compositing font glyphs, change this value to adjust contrast (Adobe and
/// Apple prefer 1.8, others may prefer 2.2). (Default = 1.8)
///
ULExport void ulConfigSetFontGamma(ULConfig config, double font_gamma);

///
/// Global user-defined CSS string (included before any CSS on the page).
///
/// You can use this to override default styles for various elements on the page.
///
/// @note This is an actual string of CSS, not a file path.
///
ULExport void ulConfigSetUserStylesheet(ULConfig config, ULString css_string);

///
/// Whether or not to continuously repaint any Views, regardless if they are dirty.
///
/// This is mainly used to diagnose painting/shader issues and profile performance.
/// 
/// (Default = False)
///
ULExport void ulConfigSetForceRepaint(ULConfig config, bool enabled);

///
/// The delay (in seconds) between every tick of a CSS animation.
///
/// (Default = 1.0 / 60.0)
///
ULExport void ulConfigSetAnimationTimerDelay(ULConfig config, double delay);

///
/// The delay (in seconds) between every tick of a smooth scroll animation.
///
/// (Default = 1.0 / 60.0)
///
ULExport void ulConfigSetScrollTimerDelay(ULConfig config, double delay);

///
/// The delay (in seconds) between every call to the recycler.
///
/// The library attempts to reclaim excess memory during calls to the internal recycler. You can
/// change how often this is run by modifying this value.
/// 
/// (Default = 4.0)
///
ULExport void ulConfigSetRecycleDelay(ULConfig config, double delay);

///
/// The size of WebCore's memory cache in bytes.
///
/// @note  You should increase this if you anticipate handling pages with large resources, Safari
///        typically uses 128+ MiB for its cache.
///
/// (Default = 64 * 1024 * 1024)
///
ULExport void ulConfigSetMemoryCacheSize(ULConfig config, unsigned int size);

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
/// (Default = 0)
///
ULExport void ulConfigSetPageCacheSize(ULConfig config, unsigned int size);

///
/// The system's physical RAM size in bytes.
///
/// JavaScriptCore tries to detect the system's physical RAM size to set reasonable allocation
/// limits. Set this to anything other than 0 to override the detected value. Size is in bytes.
///
/// This can be used to force JavaScriptCore to be more conservative with its allocation strategy
/// (at the cost of some performance).
///
ULExport void ulConfigSetOverrideRAMSize(ULConfig config, unsigned int size);

///
/// The minimum size of large VM heaps in JavaScriptCore.
///
/// Set this to a lower value to make these heaps start with a smaller initial value.
///
/// (Default = 32 * 1024 * 1024)
///
ULExport void ulConfigSetMinLargeHeapSize(ULConfig config, unsigned int size);

///
/// The minimum size of small VM heaps in JavaScriptCore.
///
/// Set this to a lower value to make these heaps start with a smaller initial value.
///
/// (Default = 1 * 1024 * 1024)
/// 
ULExport void ulConfigSetMinSmallHeapSize(ULConfig config, unsigned int size);

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
ULExport void ulConfigSetNumRendererThreads(ULConfig config, unsigned int num_renderer_threads);

///
/// The max amount of time (in seconds) to allow repeating timers to run during each call to
/// Renderer::Update.
///
/// The library will attempt to throttle timers if this time budget is exceeded.
/// 
/// (Default = 1.0 / 200.0)
///
ULExport void ulConfigSetMaxUpdateTime(ULConfig config, double max_update_time);

///
/// The alignment (in bytes) of the BitmapSurface when using the CPU renderer.
///
/// The underlying bitmap associated with each BitmapSurface will have row_bytes padded to reach
/// this alignment.
///
/// Aligning the bitmap helps improve performance when using the CPU renderer. Determining the
/// proper value to use depends on the CPU architecture and max SIMD instruction set used.
///
/// We generally target the 128-bit SSE2 instruction set across most PC platforms so '16' is a safe
/// value to use.
///
/// You can set this to '0' to perform no padding (row_bytes will always be width * 4) at a slight
/// cost to performance.
///
/// (Default = 16)
///
ULExport void ulConfigSetBitmapAlignment(ULConfig config, unsigned int bitmap_alignment);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_CONFIG_H