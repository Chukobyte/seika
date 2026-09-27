#include "pool_allocator.h"
#include "seika/assert.h"
#include <string.h>

#include "seika/math/math.h"

// Align size to proper boundary
static inline usize align_size(usize size, usize alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

SkaPool* ska_pool_init(usize block_size, usize block_count) {
    SKA_ASSERT(block_size > 0);
    SKA_ASSERT(block_count > 0);
    
    // Ensure block size is at least big enough for the free list pointer
    // and is properly aligned
    usize aligned_block_size = align_size(block_size, sizeof(void*));
    aligned_block_size = SKA_MATH_MAX(aligned_block_size, sizeof(SkaPoolBlock));

    // Allocate the pool structure
    SkaPool* pool = SKA_ALLOC(SkaPool);
    
    // Allocate the buffer for all blocks
    usize buffer_size = aligned_block_size * block_count;
    pool->buffer = ska_mem_global_allocate(buffer_size);
    pool->block_size = aligned_block_size;
    pool->block_count = block_count;
    pool->used_count = 0;
    pool->owns_buffer = true;
    
    // Initialize free list - thread all blocks together
    pool->free_list = NULL;
    for (usize i = 0; i < block_count; i++) {
        void* block = (char*)pool->buffer + (i * aligned_block_size);
        SkaPoolBlock* pool_block = (SkaPoolBlock*)block;
        pool_block->next = pool->free_list;
        pool->free_list = pool_block;
    }
    
    return pool;
}

SkaPool* ska_pool_init_from_buffer(void* buffer, usize buffer_size, usize block_size) {
    SKA_ASSERT(buffer);
    SKA_ASSERT(buffer_size > 0);
    SKA_ASSERT(block_size > 0);
    
    // Align block size
    usize aligned_block_size = align_size(block_size, sizeof(void*));
    aligned_block_size = SKA_MATH_MAX(aligned_block_size, sizeof(SkaPoolBlock));

    // Calculate how many blocks fit
    usize block_count = buffer_size / aligned_block_size;
    SKA_ASSERT_FMT(block_count > 0, "Buffer too small for even one block");
    
    // Allocate pool structure
    SkaPool* pool = SKA_ALLOC(SkaPool);
    pool->buffer = buffer;
    pool->block_size = aligned_block_size;
    pool->block_count = block_count;
    pool->used_count = 0;
    pool->owns_buffer = false;
    
    // Initialize free list
    pool->free_list = NULL;
    for (usize i = 0; i < block_count; i++) {
        void* block = (char*)pool->buffer + (i * aligned_block_size);
        SkaPoolBlock* pool_block = (SkaPoolBlock*)block;
        pool_block->next = pool->free_list;
        pool->free_list = pool_block;
    }
    
    return pool;
}

void ska_pool_deinit(SkaPool* pool) {
    if (!pool) return;
    
    if (pool->owns_buffer && pool->buffer) {
        SKA_FREE(pool->buffer);
    }
    
    SKA_FREE(pool);
}

void ska_pool_reset(SkaPool* pool) {
    SKA_ASSERT(pool);
    
    // Re-thread all blocks into free list
    pool->free_list = NULL;
    pool->used_count = 0;
    
    for (usize i = 0; i < pool->block_count; i++) {
        void* block = (char*)pool->buffer + (i * pool->block_size);
        SkaPoolBlock* pool_block = (SkaPoolBlock*)block;
        pool_block->next = pool->free_list;
        pool->free_list = pool_block;
    }
}

void* ska_pool_alloc(SkaPool* pool) {
    SKA_ASSERT(pool);
    if (!pool->free_list) return NULL; // Allocation failed, no more free blocks
    // SKA_ASSERT_REL_FMT(pool->freeList, "No more free blocks, pool is full! Used: %zu / %zu blocks", pool->usedCount, pool->blockCount);

    // Pop from free list
    void* block = pool->free_list;
    pool->free_list = pool->free_list->next;
    pool->used_count++;
    
    return block;
}

void ska_pool_free(SkaPool* pool, void* block) {
    SKA_ASSERT(pool);

    if (!block) return;

    SKA_ASSERT_FMT(ska_pool_owns_block(pool, block), "Block does not belong to this pool!");

    // Push to free list
    SkaPoolBlock* pool_block = (SkaPoolBlock*)block;
    pool_block->next = pool->free_list;
    pool->free_list = pool_block;
    pool->used_count--;
}

usize ska_pool_get_used_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->used_count;
}

usize ska_pool_get_free_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->block_count - pool->used_count;
}

usize ska_pool_get_total_count(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->block_count;
}

bool ska_pool_is_full(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->used_count >= pool->block_count;
}

bool ska_pool_is_empty(const SkaPool* pool) {
    SKA_ASSERT(pool);
    return pool->used_count == 0;
}

bool ska_pool_owns_block(const SkaPool* pool, const void* block) {
    SKA_ASSERT(pool);
    
    if (!block) return false;
    
    usize block_addr = (usize)block;
    usize buffer_start = (usize)pool->buffer;
    usize buffer_end = buffer_start + (pool->block_size * pool->block_count);
    
    // Check if address is within buffer range
    if (block_addr < buffer_start || block_addr >= buffer_end) {
        return false;
    }
    
    // Check if address is aligned to block boundary
    usize offset = block_addr - buffer_start;
    return (offset % pool->block_size) == 0;
}

// ============================================================================
// Allocator interface implementation
// ============================================================================

SkaAllocator ska_mem_pool_allocator_init(usize block_size, usize block_count) {
    SkaPool* pool = ska_pool_init(block_size, block_count);
    
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
    
    SKA_ASSERT_FMT(bytes <= pool->block_size, "Requested size %zu exceeds pool block size %zu", bytes, pool->block_size);
    
    return ska_pool_alloc(pool);
}

void* ska_mem_pool_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes) {
    void* block = ska_mem_pool_allocator_allocate(allocator, bytes);
    if (block) {
        SkaPool* pool = (SkaPool*)allocator->context;
        memset(block, 0, pool->block_size);
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
