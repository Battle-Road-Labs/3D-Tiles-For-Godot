/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_LOGGER_H
#define ULTRALIGHT_CAPI_LOGGER_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Logger
 *****************************************************************************/

typedef enum { kLogLevel_Error = 0, kLogLevel_Warning, kLogLevel_Info } ULLogLevel;

///
/// The callback invoked when the library wants to print a message to the log.
///
typedef void (*ULLoggerLogMessageCallback)(ULLogLevel log_level, ULString message);

typedef struct {
  ULLoggerLogMessageCallback log_message;
} ULLogger;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_LOGGER_H