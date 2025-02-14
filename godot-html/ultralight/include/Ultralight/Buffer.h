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

namespace ultralight {

///
/// Function signature for a user-defined destruction callback to be optionally called when Buffer
/// is destroyed.
///
/// @param  user_data  Pointer to user-defined user-data (this will be the same value as what was
///                    passed to Buffer::Create, if any)
///
/// @param  data       Pointer to raw Buffer data.
/// 
typedef void (*DestroyBufferCallback)(void* user_data, void* data);

///
/// A fixed-size byte container for passing data around.
///
class UExport Buffer : public RefCounted {
 public:
  ///
  /// Create a Buffer from existing, user-owned data without any copies. An optional, user-supplied
  /// callback will be called to deallocate data upon destruction.
  ///
  /// @param  data                  A pointer to the data.
  ///
  /// @param  size                  Size of the data in bytes.
  ///
  /// @param  user_data             Optional user data that will be passed to destruction_callback
  ///                               when the returned Buffer is destroyed.
  ///
  /// @param  destruction_callback  Optional callback that will be called upon destruction. Pass a
  ///                               null pointer if you don't want to be informed of destruction.
  ///
  ///
  /// @return  A ref-counted Buffer object that wraps the existing data.
  ///
  static RefPtr<Buffer> Create(void* data, size_t size, void* user_data,
                               DestroyBufferCallback destruction_callback);

  ///
  /// Create a Buffer from existing data, a deep copy of data will be made.
  ///
  static RefPtr<Buffer> CreateFromCopy(const void* data, size_t size);

  ///
  /// Get a pointer to the raw byte data.
  ///
  virtual void* data() = 0;

  ///
  /// Get the size in bytes.
  ///
  virtual size_t size() const = 0;

  ///
  /// Get the user data associated with this Buffer, if any.
  ///
  virtual void* user_data() = 0;

  ///
  /// Check whether this Buffer owns its own data (Buffer was created via CreateFromCopy).
  /// If this is false, Buffer will call the user-supplied destruction callback to deallocate data
  /// when this Buffer instance is destroyed.
  ///
  virtual bool owns_data() const = 0;

 protected:
  Buffer();
  virtual ~Buffer();
  Buffer(const Buffer&);
  void operator=(const Buffer&);
};

} // namespace ultralight
