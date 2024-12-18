#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

#define SKA_ALLOC(DataType)             \
(DataType*) ska_mem_allocate(sizeof(DataType))

#define SKA_ALLOC_ZEROED(DataType)             \
(DataType*) ska_mem_allocate_zeroed(sizeof(DataType))

#define SKA_ALLOC_BYTES(Bytes)             \
ska_mem_allocate(Bytes)

#define SKA_ALLOC_BYTES_ZEROED(Bytes)             \
ska_mem_allocate_zeroed(Bytes)

#define SKA_FREE(Memory)             \
ska_mem_free(Memory)

typedef struct SkaMemAllocator {
    void* (*allocate)(usize bytes);
    void* (*allocate_zeroed)(usize bytes);
    void* (*reallocate)(void* memory, usize bytes);
    void  (*free)(void* memory);
    bool  (*report_leaks)(void);
} SkaMemAllocator;

void ska_mem_set_current_allocator(const SkaMemAllocator allocator);
const SkaMemAllocator* ska_mem_get_current_allocator();
void ska_mem_reset_to_default_allocator();


// Calls using the current allocator

void* ska_mem_allocate(usize bytes);
void* ska_mem_allocate_zeroed(usize bytes);
void* ska_mem_reallocate(void* memory, usize bytes);
void ska_mem_free(void* memory);
bool ska_mem_report_leaks();

#ifdef __cplusplus
}
#endif
