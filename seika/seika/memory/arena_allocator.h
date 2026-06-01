#pragma once

#include "seika/memory.h"

typedef struct SkaArena {
    void* buffer;
    usize capacity;
    usize offset;
    bool ownsBuffer;
} SkaArena;

// Create arena with its own allocated buffer
SkaArena* ska_arena_init(usize capacity);

// Create arena from existing buffer (doesn't own it)
SkaArena* ska_arena_init_from_buffer(void* buffer, usize size);

// Destroy arena (frees buffer if owned)
void ska_arena_deinit(SkaArena* arena);

// Reset arena to beginning (keeps buffer)
void ska_arena_reset(SkaArena* arena);

// Query functions
usize ska_arena_get_used(const SkaArena* arena);
usize ska_arena_get_remaining(const SkaArena* arena);
void* ska_arena_get_current_ptr(const SkaArena* arena);

// Direct allocation functions (if you don't want to go through the interface)
void* ska_arena_alloc(SkaArena* arena, usize size);
void* ska_arena_alloc_zeroed(SkaArena* arena, usize size);
void* ska_arena_alloc_aligned(SkaArena* arena, usize size, usize alignment);

// Allocator interface

SkaAllocator ska_mem_arena_allocator_init(usize blockSize);
void ska_mem_arena_allocator_deinit(SkaAllocator allocator);

void* ska_mem_arena_allocator_allocate(SkaAllocator* allocator, usize bytes);
void* ska_mem_arena_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes);
void* ska_mem_arena_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes);
void ska_mem_arena_allocator_free(SkaAllocator* allocator, void* memory);
