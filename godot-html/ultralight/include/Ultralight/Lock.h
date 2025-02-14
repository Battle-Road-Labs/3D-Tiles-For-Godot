/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <atomic>
#include <mutex> // for std::lock_guard<>

namespace ultralight {

///
/// Tiny, efficient spinlock that is optimized for short locking periods but will still
/// intelligently yield the current thread and save CPU if the lock is held longer.
///
/// Can be used in place of std::mutex since it implements the STL's Lockable interface.
///
class Lock {
 public:
  constexpr Lock() = default;

  UL_ALWAYS_INLINE void lock() noexcept {
    // Optimistically assume the lock is free on the first try
    if (!lock_.exchange(true, std::memory_order_acquire))
      return;

    contended_lock();
  }

  UL_ALWAYS_INLINE bool try_lock() noexcept {
    // First do a relaxed load to check if lock is free in order to prevent
    // unnecessary cache misses if someone does while(!try_lock())
    return !lock_.load(std::memory_order_relaxed)
        && !lock_.exchange(true, std::memory_order_acquire);
  }

  UL_ALWAYS_INLINE void unlock() noexcept { lock_.store(false, std::memory_order_release); }

 protected:
  Lock(const Lock&) = delete;
  Lock& operator=(const Lock&) = delete;

  void contended_lock() noexcept;

  std::atomic<bool> lock_ = { 0 };
};

using LockHolder = std::lock_guard<Lock>;

} // namespace ultralight
