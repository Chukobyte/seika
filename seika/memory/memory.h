#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

#define SKA_ALLOC(DataType)             \
(DataType*)ska_mem_global_allocate(sizeof(DataType))

#define SKA_ALLOC_ZEROED(DataType)             \
(DataType*)ska_mem_global_allocate_zeroed(sizeof(DataType))

#define SKA_ALLOC_BYTES(Bytes)             \
ska_mem_global_allocate(Bytes)

#define SKA_ALLOC_BYTES_ZEROED(Bytes)             \
ska_mem_global_allocate_zeroed(Bytes)

#define SKA_ALLOC_A(Allocator, DataType)             \
(DataType*)Allocator.allocate(&Allocator, sizeof(DataType))

#define SKA_ALLOC_ZEROED_A(Allocator, DataType)             \
(DataType*)Allocator.allocate_zeroed(&Allocator, sizeof(DataType))

#define SKA_ALLOC_BYTES_A(Allocator, Bytes)             \
Allocator.allocate(&Allocator, Bytes)

#define SKA_ALLOC_BYTES_ZEROED_A(Allocator, Bytes)             \
Allocator.allocate_zeroed(&Allocator, Bytes)

#define SKA_REALLOC(Memory, Bytes)             \
ska_mem_global_reallocate(Memory, Bytes)

#define SKA_REALLOC_A(Allocator, Memory, Bytes)             \
Allocator.reallocate(&Allocator, Memory, Bytes)

#define SKA_FREE(Memory)             \
ska_mem_global_free(Memory)

#define SKA_FREE_A(Allocator, Memory)             \
Allocator.free(&Allocator, Memory)

typedef struct SkaAllocator {
    void* context;
    void* (*allocate)(struct SkaAllocator* allocator, usize bytes);
    void* (*allocate_zeroed)(struct SkaAllocator* allocator, usize bytes);
    void* (*reallocate)(struct SkaAllocator* allocator, void* memory, usize bytes);
    void  (*free)(struct SkaAllocator* allocator, void* memory);
} SkaAllocator;

void ska_mem_set_global_allocator(SkaAllocator allocator);
SkaAllocator ska_mem_get_global_allocator();
void ska_mem_reset_to_default_global_allocator();

SkaAllocator ska_mem_standard_allocator();

// Calls using the currently set global allocator

void* ska_mem_global_allocate(usize bytes);
void* ska_mem_global_allocate_zeroed(usize bytes);
void* ska_mem_global_reallocate(void* memory, usize bytes);
void ska_mem_global_free(void* memory);

#ifdef __cplusplus
}
#endif
