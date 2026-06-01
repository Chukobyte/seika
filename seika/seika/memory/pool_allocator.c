#include "pool_allocator.h"
#include "seika/assert.h"
#include <string.h>

#include "seika/math/math.h"

// Align size to proper boundary
static inline usize align_size(usize size, usize alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

SkaPool* ska_pool_init(usize blockSize, usize blockCount) {
    SKA_ASSERT(blockSize > 0);
    SKA_ASSERT(blockCount > 0);
    
    // Ensure block size is at least big enough for the free list pointer
    // and is properly aligned
    usize alignedBlockSize = align_size(blockSize, sizeof(void*));
    alignedBlockSize = SKA_MATH_MAX(alignedBlockSize, sizeof(SkaPoolBlock));

    // Allocate the pool structure
    SkaPool* pool = SKA_ALLOC(SkaPool);
    
    // Allocate the buffer for all blocks
    usize bufferSize = alignedBlockSize * blockCount;
    pool->buffer = ska_mem_global_allocate(bufferSize);
    pool->blockSize = alignedBlockSize;
    pool->blockCount = blockCount;
    pool->usedCount = 0;
    pool->ownsBuffer = true;
    
    // Initialize free list - thread all blocks together
    pool->freeList = NULL;
    for (usize i = 0; i < blockCount; i++) {
        void* block = (char*)pool->buffer + (i * alignedBlockSize);
        SkaPoolBlock* poolBlock = (SkaPoolBlock*)block;
        poolBlock->next = pool->freeList;
        pool->freeList = poolBlock;
    }
    
    return pool;
}

SkaPool* ska_pool_init_from_buffer(void* buffer, usize bufferSize, usize blockSize) {
    SKA_ASSERT(buffer);
    SKA_ASSERT(bufferSize > 0);
    SKA_ASSERT(blockSize > 0);
    
    // Align block size
    usize alignedBlockSize = align_size(blockSize, sizeof(void*));
    alignedBlockSize = SKA_MATH_MAX(alignedBlockSize, sizeof(SkaPoolBlock));

    // Calculate how many blocks fit
    usize blockCount = bufferSize / alignedBlockSize;
    SKA_ASSERT_FMT(blockCount > 0, "Buffer too small for even one block");
    
    // Allocate pool structure
    SkaPool* pool = SKA_ALLOC(SkaPool);
    pool->buffer = buffer;
    pool->blockSize = alignedBlockSize;
    pool->blockCount = blockCount;
    pool->usedCount = 0;
    pool->ownsBuffer = false;
    
    // Initialize free list
    pool->freeList = NULL;
    for (usize i = 0; i < blockCount; i++) {
        void* block = (char*)pool->buffer + (i * alignedBlockSize);
        SkaPoolBlock* poolBlock = (SkaPoolBlock*)block;
        poolBlock->next = pool->freeList;
        pool->freeList = poolBlock;
    }
    
    return pool;
}

void ska_pool_deinit(SkaPool* pool) {
    if (!pool) return;
    
    if (pool->ownsBuffer && pool->buffer) {
        SKA_FREE(pool->buffer);
    }
    
    SKA_FREE(pool);
}

void ska_pool_reset(SkaPool* pool) {
    SKA_ASSERT(pool);
    
    // Re-thread all blocks into free list
    pool->freeList = NULL;
    pool->usedCount = 0;
    
    for (usize i = 0; i < pool->blockCount; i++) {
        void* block = (char*)pool->buffer + (i * pool->blockSize);
        SkaPoolBlock* poolBlock = (SkaPoolBlock*)block;
        poolBlock->next = pool->freeList;
        pool->freeList = poolBlock;
    }
}

void* ska_pool_alloc(SkaPool* pool) {
    SKA_ASSERT(pool);
    if (!pool->freeList) return NULL; // Allocation failed, no more free blocks
    // SKA_ASSERT_REL_FMT(pool->freeList, "No more free blocks, pool is full! Used: %zu / %zu blocks", pool->usedCount, pool->blockCount);

    // Pop from free list
    void* block = pool->freeList;
    pool->freeList = pool->freeList->next;
    pool->usedCount++;
    
    return block;
}

void ska_pool_free(SkaPool* pool, void* block) {
    SKA_ASSERT(pool);

    if (!block) return;

    SKA_ASSERT_FMT(ska_pool_owns_block(pool, block), "Block does not belong to this pool!");

    // Push to free list
    SkaPoolBlock* poolBlock = (SkaPoolBlock*)block;
    poolBlock->next = pool->freeList;
    pool->freeList = poolBlock;
    pool->usedCount--;
}

usize ska_pool_get_used_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->usedCount;
}

usize ska_pool_get_free_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->blockCount - pool->usedCount;
}

usize ska_pool_get_total_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->blockCount;
}

bool ska_pool_is_full(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->usedCount >= pool->blockCount;
}

bool ska_pool_is_empty(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->usedCount == 0;
}

bool ska_pool_owns_block(const SkaPool* pool, const void* block) {
    SKA_ASSERT(pool);
    
    if (!block) return false;
    
    usize blockAddr = (usize)block;
    usize bufferStart = (usize)pool->buffer;
    usize bufferEnd = bufferStart + (pool->blockSize * pool->blockCount);
    
    // Check if address is within buffer range
    if (blockAddr < bufferStart || blockAddr >= bufferEnd) {
        return false;
    }
    
    // Check if address is aligned to block boundary
    usize offset = blockAddr - bufferStart;
    return (offset % pool->blockSize) == 0;
}

// ============================================================================
// Allocator interface implementation
// ============================================================================

SkaAllocator ska_mem_pool_allocator_init(usize blockSize, usize blockCount) {
    SkaPool* pool = ska_pool_init(blockSize, blockCount);
    
    SkaAllocator allocator = {
        .context = pool,
        .allocate = ska_mem_pool_allocator_allocate,
        .allocate_zeroed = ska_mem_pool_allocator_allocate_zeroed,
        .reallocate = ska_mem_pool_allocator_reallocate,
        .free = ska_mem_pool_allocator_free
    };
    
    return allocator;
}

void ska_mem_pool_allocator_deinit(SkaAllocator allocator) {
    if (allocator.context) {
        ska_pool_deinit((SkaPool*)allocator.context);
    }
}

void* ska_mem_pool_allocator_allocate(SkaAllocator* allocator, usize bytes) {
    SKA_ASSERT(allocator);
    SKA_ASSERT(allocator->context);
    
    SkaPool* pool = (SkaPool*)allocator->context;
    
    SKA_ASSERT_FMT(bytes <= pool->blockSize, "Requested size %zu exceeds pool block size %zu", bytes, pool->blockSize);
    
    return ska_pool_alloc(pool);
}

void* ska_mem_pool_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes) {
    void* block = ska_mem_pool_allocator_allocate(allocator, bytes);
    if (block) {
        SkaPool* pool = (SkaPool*)allocator->context;
        memset(block, 0, pool->blockSize);
    }
    return block;
}

void* ska_mem_pool_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes) {
    SKA_ASSERT_FMT(false, "Pool allocator does not support reallocation");
    return NULL;
}

void ska_mem_pool_allocator_free(SkaAllocator* allocator, void* memory) {
    SKA_ASSERT(allocator);
    SKA_ASSERT(allocator->context);
    
    SkaPool* pool = (SkaPool*)allocator->context;
    ska_pool_free(pool, memory);
}

void ska_mem_pool_allocator_reset(SkaAllocator* allocator) {
    SKA_ASSERT(allocator);
    SKA_ASSERT(allocator->context);
    
    SkaPool* pool = (SkaPool*)allocator->context;
    ska_pool_reset(pool);
}

usize ska_mem_pool_allocator_get_used_count(SkaAllocator* allocator) {
    SKA_ASSERT(allocator);
    SKA_ASSERT(allocator->context);
    
    SkaPool* pool = (SkaPool*)allocator->context;
    return ska_pool_get_used_count(pool);
}
