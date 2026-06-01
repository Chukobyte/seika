#pragma once

#include "seika/defines.h"
#include "seika/memory.h"

typedef struct SkaPoolBlock {
    struct SkaPoolBlock* next;
} SkaPoolBlock;

typedef struct SkaPool {
    void* buffer;
    usize blockSize;
    usize blockCount;
    usize usedCount;
    SkaPoolBlock* freeList;
    bool ownsBuffer;
} SkaPool;

// Create pool with its own allocated buffer
SkaPool* ska_pool_init(usize blockSize, usize blockCount);
// Create pool from existing buffer (doesn't own it)
SkaPool* ska_pool_init_from_buffer(void* buffer, usize bufferSize, usize blockSize);
// Destroy pool (frees buffer if owned)
void ska_pool_deinit(SkaPool* pool);
// Reset pool (marks all blocks as free)
void ska_pool_reset(SkaPool* pool);
// Allocate a block from the pool
void* ska_pool_alloc(SkaPool* pool);
// Free a block back to the pool
void ska_pool_free(SkaPool* pool, void* block);

// Query functions
usize ska_pool_get_used_count(const SkaPool* pool);
usize ska_pool_get_free_count(const SkaPool* pool);
usize ska_pool_get_total_count(const SkaPool* pool);
bool ska_pool_is_full(const SkaPool* pool);
bool ska_pool_is_empty(const SkaPool* pool);

// Check if a pointer belongs to this pool
bool ska_pool_owns_block(const SkaPool* pool, const void* block);

// ============================================================================
// Allocator interface
// ============================================================================

// Create a pool allocator that can be used with the SkaAllocator interface
SkaAllocator ska_mem_pool_allocator_init(usize blockSize, usize blockCount);

// Destroy a pool allocator
void ska_mem_pool_allocator_deinit(SkaAllocator allocator);

// SkaAllocator function pointers (internal - use through the interface)
void* ska_mem_pool_allocator_allocate(SkaAllocator* allocator, usize bytes);
void* ska_mem_pool_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes);
void* ska_mem_pool_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes);
void ska_mem_pool_allocator_free(SkaAllocator* allocator, void* memory);

// Helper functions for pool-specific operations through the allocator interface
void ska_mem_pool_allocator_reset(SkaAllocator* allocator);
usize ska_mem_pool_allocator_get_used_count(SkaAllocator* allocator);
