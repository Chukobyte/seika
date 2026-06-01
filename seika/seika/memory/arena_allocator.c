#include "arena_allocator.h"

#include "seika/assert.h"

// Align pointer forward to given alignment
static inline usize align_forward(usize ptr, usize alignment) {
    usize modulo = ptr & (alignment - 1);
    if (modulo != 0) {
        ptr += alignment - modulo;
    }
    return ptr;
}

SkaArena* ska_arena_init(usize capacity) {
    SKA_ASSERT(capacity > 0);

    SkaArena* arena = SKA_ALLOC(SkaArena);
    arena->buffer = ska_mem_global_allocate(capacity);
    arena->capacity = capacity;
    arena->offset = 0;
    arena->ownsBuffer = true;

    return arena;
}

SkaArena* ska_arena_init_from_buffer(void* buffer, usize size) {
    SKA_ASSERT(buffer);
    SKA_ASSERT(size > 0);

    SkaArena* arena = SKA_ALLOC(SkaArena);
    arena->buffer = buffer;
    arena->capacity = size;
    arena->offset = 0;
    arena->ownsBuffer = false;

    return arena;
}

void ska_arena_deinit(SkaArena* arena) {
    if (!arena) return;

    if (arena->ownsBuffer && arena->buffer) {
        SKA_FREE(arena->buffer);
    }

    SKA_FREE(arena);
}

void ska_arena_reset(SkaArena* arena) {
    SKA_ASSERT(arena);
    arena->offset = 0;
}

usize ska_arena_get_used(const SkaArena* arena) {
    SKA_ASSERT(arena);
    return arena->offset;
}

usize ska_arena_get_remaining(const SkaArena* arena) {
    SKA_ASSERT(arena);
    return arena->capacity - arena->offset;
}

void* ska_arena_get_current_ptr(const SkaArena* arena) {
    SKA_ASSERT(arena);
    return (void*)((usize)arena->buffer + arena->offset);
}

void* ska_arena_alloc(SkaArena* arena, usize size) {
    SKA_ASSERT(arena);
    SKA_ASSERT(arena->buffer);

    if (size == 0) return NULL;

    // Default 8-byte alignment
    usize currentPtr = (usize)arena->buffer + arena->offset;
    usize alignedPtr = align_forward(currentPtr, 8);
    usize alignmentOffset = alignedPtr - currentPtr;

    // Check space
    if (arena->offset + alignmentOffset + size > arena->capacity) {
        SKA_ASSERT_REL_FMT(false,
            "Arena out of memory! Requested: %zu bytes, Available: %zu bytes",
            size, arena->capacity - arena->offset);
        return NULL;
    }

    void* ptr = (void*)alignedPtr;
    arena->offset += alignmentOffset + size;

    return ptr;
}

void* ska_arena_alloc_zeroed(SkaArena* arena, usize size) {
    void* ptr = ska_arena_alloc(arena, size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

// TODO: Figure out when to add alignment param for allocator interface
void* ska_arena_alloc_aligned(SkaArena* arena, usize size, usize alignment) {
    SKA_ASSERT(arena);
    SKA_ASSERT(arena->buffer);
    SKA_ASSERT((alignment & (alignment - 1)) == 0); // Must be power of 2

    if (size == 0) return NULL;

    usize currentPtr = (usize)arena->buffer + arena->offset;
    usize alignedPtr = align_forward(currentPtr, alignment);
    usize alignmentOffset = alignedPtr - currentPtr;

    if (arena->offset + alignmentOffset + size > arena->capacity) {
        SKA_ASSERT_REL_FMT(false,
            "Arena out of memory! Requested: %zu bytes (aligned to %zu), Available: %zu bytes",
            size, alignment, arena->capacity - arena->offset);
        return NULL;
    }

    void* ptr = (void*)alignedPtr;
    arena->offset += alignmentOffset + size;

    return ptr;
}

// Allocator interface

SkaAllocator ska_mem_arena_allocator_init(usize blockSize) {
    SkaArena* arena = ska_arena_init(blockSize);

    SkaAllocator allocator = {
        .context = arena,
        .allocate = ska_mem_arena_allocator_allocate,
        .allocate_zeroed = ska_mem_arena_allocator_allocate_zeroed,
        .reallocate = ska_mem_arena_allocator_reallocate,
        .free = ska_mem_arena_allocator_free
    };

    return allocator;
}
void ska_mem_arena_allocator_deinit(SkaAllocator allocator) {
    SkaArena* arena = (SkaArena*)allocator.context;
    ska_arena_deinit(arena);
}

void* ska_mem_arena_allocator_allocate(SkaAllocator* allocator, usize bytes) {
    SkaArena* arena = (SkaArena*)allocator->context;
    return ska_arena_alloc(arena, bytes);
}

void* ska_mem_arena_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes) {
    SkaArena* arena = (SkaArena*)allocator->context;
    return ska_arena_alloc_zeroed(arena, bytes);
}

void* ska_mem_arena_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes) {
    // TODO: Determine if we want to do something else...
    SKA_ASSERT_FMT(false, "Arena allocator does not support reallocation");
    return NULL;
}

// No-op
void ska_mem_arena_allocator_free(SkaAllocator* allocator, void* memory) {}
