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

// Unique id of the thread, used for referencing the created thread later.
//   * on Windows this should match the thread identifier returned by either
//       _beginthreadex() or GetCurrentThreadId()
//   * on POSIX this can be whatever unique id you want
typedef uint32_t ThreadId;

// Platform-specific handle
//   * on Windows this is HANDLE
//   * on POSIX this is pthread_t
typedef uint64_t ThreadHandle;

// Entry point for the thread, this function should be called by the thread once it is active
// and should be passed entry_point_data as the argument.
typedef void (*ThreadEntryPoint)(void*);

// The type of thread, you can choose to optionally handle these for better performance
enum class ThreadType : uint8_t {
  Unknown = 0,
  JavaScript,
  Compiler,
  GarbageCollection,
  Network,
  Graphics,
  Audio,
};

struct UExport CreateThreadResult {
  ThreadId id;
  ThreadHandle handle;
};

class UExport ThreadFactory {
 public:
  virtual ~ThreadFactory() = default;

  virtual bool CreateThread(const char* name, ThreadType type, ThreadEntryPoint entry_point,
                            void* entry_point_data, CreateThreadResult& result) = 0;
};

} // namespace ultralight
