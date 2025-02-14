/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_DEFINES_H
#define ULTRALIGHT_CAPI_DEFINES_H

#include <stddef.h>
#include <stdint.h>
#include <JavaScriptCore/JavaScript.h>
#ifdef __OBJC__
#import <AppKit/NSEvent.h>
#endif

#if defined(ULTRALIGHT_STATIC_BUILD)
#define ULExport
#else
#if defined(__WIN32__) || defined(_WIN32)
#if defined(ULTRALIGHT_IMPLEMENTATION)
#define ULExport __declspec(dllexport)
#else
#define ULExport __declspec(dllimport)
#endif
#else
#define ULExport __attribute__((visibility("default")))
#endif
#endif

#if defined(__WIN32__) || defined(_WIN32)
#define _thread_local __declspec(thread)
#ifndef _NATIVE_WCHAR_T_DEFINED
#define DISABLE_NATIVE_WCHAR_T
typedef unsigned short ULChar16;
#else
typedef wchar_t ULChar16;
#endif
#else
#define _thread_local __thread
typedef unsigned short ULChar16;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct C_Config* ULConfig;
typedef struct C_Renderer* ULRenderer;
typedef struct C_Session* ULSession;
typedef struct C_ViewConfig* ULViewConfig;
typedef struct C_View* ULView;
typedef struct C_Bitmap* ULBitmap;
typedef struct C_String* ULString;
typedef struct C_Buffer* ULBuffer;
typedef struct C_KeyEvent* ULKeyEvent;
typedef struct C_MouseEvent* ULMouseEvent;
typedef struct C_ScrollEvent* ULScrollEvent;
typedef struct C_GamepadEvent* ULGamepadEvent;
typedef struct C_GamepadAxisEvent* ULGamepadAxisEvent;
typedef struct C_GamepadButtonEvent* ULGamepadButtonEvent;
typedef struct C_Surface* ULSurface;
typedef struct C_Surface* ULBitmapSurface;
typedef struct C_FontFile* ULFontFile;

typedef enum {
  kMessageSource_XML = 0,
  kMessageSource_JS,
  kMessageSource_Network,
  kMessageSource_ConsoleAPI,
  kMessageSource_Storage,
  kMessageSource_AppCache,
  kMessageSource_Rendering,
  kMessageSource_CSS,
  kMessageSource_Security,
  kMessageSource_ContentBlocker,
  kMessageSource_Other,
} ULMessageSource;

typedef enum {
  kMessageLevel_Log = 1,
  kMessageLevel_Warning = 2,
  kMessageLevel_Error = 3,
  kMessageLevel_Debug = 4,
  kMessageLevel_Info = 5,
} ULMessageLevel;

typedef enum {
  kCursor_Pointer = 0,
  kCursor_Cross,
  kCursor_Hand,
  kCursor_IBeam,
  kCursor_Wait,
  kCursor_Help,
  kCursor_EastResize,
  kCursor_NorthResize,
  kCursor_NorthEastResize,
  kCursor_NorthWestResize,
  kCursor_SouthResize,
  kCursor_SouthEastResize,
  kCursor_SouthWestResize,
  kCursor_WestResize,
  kCursor_NorthSouthResize,
  kCursor_EastWestResize,
  kCursor_NorthEastSouthWestResize,
  kCursor_NorthWestSouthEastResize,
  kCursor_ColumnResize,
  kCursor_RowResize,
  kCursor_MiddlePanning,
  kCursor_EastPanning,
  kCursor_NorthPanning,
  kCursor_NorthEastPanning,
  kCursor_NorthWestPanning,
  kCursor_SouthPanning,
  kCursor_SouthEastPanning,
  kCursor_SouthWestPanning,
  kCursor_WestPanning,
  kCursor_Move,
  kCursor_VerticalText,
  kCursor_Cell,
  kCursor_ContextMenu,
  kCursor_Alias,
  kCursor_Progress,
  kCursor_NoDrop,
  kCursor_Copy,
  kCursor_None,
  kCursor_NotAllowed,
  kCursor_ZoomIn,
  kCursor_ZoomOut,
  kCursor_Grab,
  kCursor_Grabbing,
  kCursor_Custom
} ULCursor;

typedef enum {
  ///
  /// Alpha channel only, 8-bits per pixel.
  ///
  /// Encoding: 8-bits per channel, unsigned normalized.
  ///
  /// Color-space: Linear (no gamma), alpha-coverage only.
  ///
  kBitmapFormat_A8_UNORM,

  ///
  /// Blue Green Red Alpha channels, 32-bits per pixel.
  ///
  /// Encoding: 8-bits per channel, unsigned normalized.
  ///
  /// Color-space: sRGB gamma with premultiplied linear alpha channel.
  ///
  kBitmapFormat_BGRA8_UNORM_SRGB
} ULBitmapFormat;

typedef enum {
  ///
  /// Key-Down event type. (Does not trigger accelerator commands in WebCore)
  ///
  /// @NOTE: You should probably use RawKeyDown instead when a physical key
  ///        is pressed. This member is only here for historic compatibility
  ///        with WebCore's key event types.
  ///
  kKeyEventType_KeyDown,

  ///
  /// Key-Up event type. Use this when a physical key is released.
  ///
  kKeyEventType_KeyUp,

  ///
  /// Raw Key-Down type. Use this when a physical key is pressed.
  ///
  /// @NOTE: You should use RawKeyDown for physical key presses since it
  ///        allows WebCore to do additional command translation.
  ///
  kKeyEventType_RawKeyDown,

  ///
  /// Character input event type. Use this when the OS generates text from
  /// a physical key being pressed (eg, WM_CHAR on Windows).
  ///
  kKeyEventType_Char,
} ULKeyEventType;

typedef enum {
  kMouseEventType_MouseMoved,
  kMouseEventType_MouseDown,
  kMouseEventType_MouseUp,
} ULMouseEventType;

typedef enum {
  kMouseButton_None = 0,
  kMouseButton_Left,
  kMouseButton_Middle,
  kMouseButton_Right,
} ULMouseButton;

typedef enum {
  kScrollEventType_ScrollByPixel,
  kScrollEventType_ScrollByPage,
} ULScrollEventType;

typedef enum {
  kGamepadEventType_Connected,
  kGamepadEventType_Disconnected,
} ULGamepadEventType;

typedef enum {
  kFaceWinding_Clockwise,
  kFaceWinding_CounterClockwise,
} ULFaceWinding;

typedef enum {
  ///
  /// Lighter hinting algorithm-- glyphs are slightly fuzzier but better
  /// resemble their original shape. This is achieved by snapping glyphs to the
  /// pixel grid only vertically which better preserves inter-glyph spacing.
  ///
  kFontHinting_Smooth,

  ///
  /// Default hinting algorithm-- offers a good balance between sharpness and
  /// shape at smaller font sizes.
  ///
  kFontHinting_Normal,

  ///
  /// Strongest hinting algorithm-- outputs only black/white glyphs. The result
  /// is usually unpleasant if the underlying TTF does not contain hints for
  /// this type of rendering.
  ///
  kFontHinting_Monochrome,
} ULFontHinting;

typedef struct {
  float left;
  float top;
  float right;
  float bottom;
} ULRect;

typedef struct {
  int left;
  int top;
  int right;
  int bottom;
} ULIntRect;

typedef struct {
  bool is_empty;
  unsigned int width;
  unsigned int height;
  unsigned int texture_id;
  unsigned int texture_width;
  unsigned int texture_height;
  ULBitmapFormat texture_format;
  ULRect uv_coords;
  unsigned int render_buffer_id;
} ULRenderTarget;

/******************************************************************************
 * Version
 *****************************************************************************/

///
/// Get the version string of the library in MAJOR.MINOR.PATCH format.
///
ULExport const char* ulVersionString();

///
/// Get the numeric major version of the library.
///
ULExport unsigned int ulVersionMajor();

///
/// Get the numeric minor version of the library.
///
ULExport unsigned int ulVersionMinor();

///
/// Get the numeric patch version of the library.
///
ULExport unsigned int ulVersionPatch();

///
/// Get the full WebKit version string.
/// 
ULExport const char* ulWebKitVersionString();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_DEFINES_H