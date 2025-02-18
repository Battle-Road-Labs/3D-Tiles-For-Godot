/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_ALLOCATOR_H
#define ULTRALIGHT_ALLOCATOR_H

#include <stddef.h>
#include <stdbool.h>
#include <Ultralight/Exports.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Global allocator interface declaration.
// 
// @see ulAllocator
//
typedef struct {
  //
  // Allocate a block of memory of at least |bytes| size.
  //
  void* (*malloc)(size_t bytes);

  //
  // Reallocate a block of memory to at least |bytes| size.
  //
  void* (*realloc)(void* address, size_t bytes);

  //
  // Free a block of memory allocated with malloc or realloc.
  //
  void (*free)(void* address);

  //
  // Allocate a block of memory of at least |bytes| size, aligned to |alignment|.
  //
  void* (*aligned_malloc)(size_t bytes, size_t alignment);

  //
  // Reallocate a block of memory to at least |bytes| size, aligned to |alignment|.
  //
  void* (*aligned_realloc)(void* address, size_t bytes, size_t alignment);

  //
  // Free a block of memory allocated with aligned_malloc or aligned_realloc.
  //
  void (*aligned_free)(void* address);

  //
  // Get the size of the memory block that backs the allocation at |address|. The memory
  // block size is always at least as large as the allocation it backs, and may be larger.
  //  * Windows equivalent: _msize
  //  * POSIX equivalent: malloc_size
  //
  size_t (*get_size_estimate)(void* address);

} ULAllocator;

//
// Get the allocator interface for the library.
//
// The C functions set in this object will be used for allocating memory inside the library
// when the UL_ENABLE_ALLOCATOR_OVERRIDE build option is enabled.
// 
// Default functions are already set for all of these but you can override them with your own.
// 
// Platform specific notes:
//    * Windows: The default functions use HeapAlloc/HeapReAlloc/HeapFree.
// 
extern UCExport ULAllocator ulAllocator;

#ifdef _WIN32
//
// Get the handle to the private heap used by the library.
// 
// This is the handle returned by HeapCreate(), you should destroy it after unloading the library
// by calling HeapDestroy().
// 
// This is only valid if the UL_ENABLE_ALLOCATOR_OVERRIDE build option is enabled and the default
// functions are set in the ulAllocator object.
// 
UCExport void* ulGetHeapHandle();
#endif

#ifdef __cplusplus
}
#endif

#endif // ULTRALIGHT_ALLOCATOR_H