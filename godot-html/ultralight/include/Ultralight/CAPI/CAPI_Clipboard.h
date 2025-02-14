/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_CLIPBOARD_H
#define ULTRALIGHT_CAPI_CLIPBOARD_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Clipboard
 *****************************************************************************/

///
/// The callback invoked when the library wants to clear the system's clipboard.
///
typedef void (*ULClipboardClearCallback)();

///
/// The callback invoked when the library wants to read from the system's clipboard.
///
/// You should store the result (if any) in 'result'.
///
typedef void (*ULClipboardReadPlainTextCallback)(ULString result);

///
/// The callback invoked when the library wants to write to the system's clipboard.
///
typedef void (*ULClipboardWritePlainTextCallback)(ULString text);

typedef struct {
  ULClipboardClearCallback clear;
  ULClipboardReadPlainTextCallback read_plain_text;
  ULClipboardWritePlainTextCallback write_plain_text;
} ULClipboard;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_CLIPBOARD_H