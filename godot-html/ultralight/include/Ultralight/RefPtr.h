/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
/*
* Portions of the below code are derived from 'RefPtr.h' from Apple's WTF,
* with the following license header:
*
* Copyright (C) 2013-2014 Apple Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <Ultralight/Defines.h>
#include <assert.h>
#include <utility>

namespace ultralight {
  
///
/// @brief  Interface for all ref-counted objects that will be managed using
///         the RefPtr<> smart pointer.
///
class UExport RefCounted {
 public:
  virtual void AddRef() const = 0;
  virtual void Release() const = 0;
  virtual int ref_count() const = 0;
 protected:
  virtual ~RefCounted();
};

template<typename T> class RefPtr;

///
/// @brief  Helper for wrapping new objects with the RefPtr smart pointer.
///
/// All ref-counted object are created with an initial ref-count of '1'.
/// The AdoptRef() helper returns a RefPtr<T> without calling AddRef().
/// This is used for creating new objects, like so:
///
///   RefPtr<Object> ref = AdoptRef(*new ObjectImpl());
///
template<typename T>
RefPtr<T> AdoptRef(T& reference)
{
  return RefPtr<T>(reference, RefPtr<T>::Adopt);
}

///
/// @brief  A nullable smart pointer.
///
/// This smart pointer automatically manages the lifetime of a RefCounted
/// object. The managed instance may be NULL.
///
template<typename T> class RefPtr {
 public:
  ///
  /// Construct a NULL ref-pointer.
  ///
  constexpr RefPtr() 
    : instance_(nullptr)
  {
  }

  ///
  /// Construct a NULL ref-pointer.
  ///
  inline RefPtr(std::nullptr_t)
    : instance_(nullptr)
  {
  }

  ///
  /// Construct from a pointer. (Will increment ref-count by one)
  ///
  inline RefPtr(T* other)
    : instance_(other)
  {
    if (instance_)
      instance_->AddRef();
  }

  ///
  /// Copy constructor.
  ///
  inline RefPtr(const RefPtr& other) 
    : instance_(other.instance_) 
  {
    if (instance_)
      instance_->AddRef();
  }

  ///
  /// Copy constructor with internal type conversion.
  ///
  template<typename U>
  RefPtr(const RefPtr<U>& other) 
    : instance_(other.instance_) 
  {
    if (instance_)
      instance_->AddRef();
  }

  ///
  /// Move constructor.
  ///
  inline RefPtr(RefPtr&& other) 
    : instance_(other.LeakRef())
  {
  }

  ///
  /// Move constructor.
  ///
  template<typename U>
  RefPtr(RefPtr<U>&& other) 
    : instance_(other.LeakRef())
  {
  }

  ///
  /// Destroy RefPtr (wll decrement ref-count by one)
  ///
  inline ~RefPtr()
  {
    T* old_value = std::move(instance_);
    instance_ = std::forward<T*>(nullptr);
    if (old_value)
      old_value->Release();
  }

  ///
  /// Get a pointer to wrapped object.
  ///
  inline T* get() const { return instance_; }

  T* LeakRef() {
    T* result = std::move(instance_);
    instance_ = std::forward<T*>(nullptr);
    return result;
  }

  T& operator*() const { assert(instance_); return *instance_; }
  inline T* operator->() const { return instance_; }

  bool operator!() const { return !instance_; }

  // This conversion operator allows implicit conversion to bool but not to other integer types.
  typedef T* (RefPtr::*UnspecifiedBoolType);
  operator UnspecifiedBoolType() const { return instance_ ? &RefPtr::instance_ : nullptr; }

  RefPtr& operator=(const RefPtr&);
  RefPtr& operator=(T*);
  RefPtr& operator=(std::nullptr_t);
  template<typename U> RefPtr& operator=(const RefPtr<U>&);
  RefPtr& operator=(RefPtr&&);
  template<typename U> RefPtr& operator=(RefPtr<U>&&);

  friend inline bool operator==(const RefPtr& a, const RefPtr& b) {
    return a.instance_ == b.instance_;
  }

  friend inline bool operator!=(const RefPtr& a, const RefPtr& b) {
    return a.instance_ != b.instance_;
  }

  friend inline bool operator<(const RefPtr& a, const RefPtr& b) {
    return a.instance_ < b.instance_;
  }

  ///
  /// Releases the ownership of the managed object, if any
  ///
  void reset();

  ///
  /// Replaces the managed object with another.
  ///
  void reset(T* obj);

  ///
  /// Exchanges the stored pointer values and the ownerships of *this and ptr.
  /// Reference counts, if any, are not adjusted.
  ///
  void swap(RefPtr& ptr);

protected:
  friend RefPtr AdoptRef<T>(T&);

  enum AdoptTag { Adopt };
  RefPtr(T& object, AdoptTag) : instance_(&object) { }

private:
  T* instance_;
};

template<typename T>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr& other)
{
  RefPtr ptr = other;
  swap(ptr);
  return *this;
}

template<typename T>
template<typename U>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr<U>& other)
{
  RefPtr ptr = other;
  swap(ptr);
  return *this;
}

template<typename T>
RefPtr<T>& RefPtr<T>::operator=(T* object)
{
  RefPtr ptr = object;
  swap(ptr);
  return *this;
}

template<typename T>
RefPtr<T>& RefPtr<T>::operator=(std::nullptr_t)
{
  T* old_instance = std::move(instance_);
  instance_ = std::forward<T*>(nullptr);
  if (old_instance)
    old_instance->Release();
  return *this;
}

template<typename T>
RefPtr<T>& RefPtr<T>::operator=(RefPtr&& other)
{
  RefPtr ptr = std::move(other);
  swap(ptr);
  return *this;
}

template<typename T>
template<typename U>
RefPtr<T>& RefPtr<T>::operator=(RefPtr<U>&& other)
{
  RefPtr ptr = std::move(other);
  swap(ptr);
  return *this;
}

template <typename T> void RefPtr<T>::reset() { *this = nullptr; }

template <typename T> void RefPtr<T>::reset(T* obj) { *this = obj; }

template<typename T>
void RefPtr<T>::swap(RefPtr& other)
{
  std::swap(instance_, other.instance_);
}

template<class T>
void swap(RefPtr<T>& a, RefPtr<T>& b)
{
  a.swap(b);
}

template<typename T, typename U>
bool operator==(const RefPtr<T>& a, const RefPtr<U>& b)
{
  return a.get() == b.get();
}

template<typename T, typename U>
bool operator==(const RefPtr<T>& a, const U* b)
{
  return a.get() == b;
}

template<typename T, typename U>
bool operator==(const T* a, const RefPtr<U>& b)
{
  return a == b.get();
}

template<typename T, typename U>
bool operator!=(const RefPtr<T>& a, const RefPtr<U>& b)
{
  return a.get() != b.get();
}

template<typename T, typename U>
bool operator!=(const RefPtr<T>& a, const U* b)
{
  return a.get() != b;
}

template<typename T, typename U>
bool operator!=(const T* a, const RefPtr<U>& b)
{
  return a != b.get();
}

}  // namespace ultralight
