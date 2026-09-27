#include "memory.h"
#include "seika/assert.h"

#pragma region
// Internal standard allocator implementation

static bool isAllocatorValid(const SkaAllocator* allocator) {
    return allocator->allocate
        && allocator->allocate_zeroed
        && allocator->reallocate
        && allocator->free;
}

static void* ska_mem_standard_allocator_allocate(SkaAllocator* allocator, usize bytes) {
    void* memory = malloc(bytes);
    SKA_ASSERT_REL_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

static void* ska_mem_standard_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes) {
    void* memory = calloc(1, bytes);
    SKA_ASSERT_REL_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

static void* ska_mem_standard_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes) {
  void *reallocated_memory = realloc(memory, bytes);
  SKA_ASSERT_REL_FMT(reallocated_memory,
                     "Out of memory or realloc failed!, size = %d", bytes);
  return reallocated_memory;
}

static void ska_mem_standard_allocator_free(SkaAllocator* allocator, void* memory) {
    free(memory);
}

// End standard memory allocation
#pragma endregion

static const SkaAllocator standard_allocator = {
    .context = NULL,
    .allocate = ska_mem_standard_allocator_allocate,
    .allocate_zeroed = ska_mem_standard_allocator_allocate_zeroed,
    .reallocate = ska_mem_standard_allocator_reallocate,
    .free = ska_mem_standard_allocator_free,
};

static SkaAllocator current_alloc = {0};

void ska_mem_set_global_allocator(const SkaAllocator allocator) {
    SKA_ASSERT_FMT(isAllocatorValid(&allocator), "Must implement all allocator functions before setting");
    current_alloc = allocator;
}

SkaAllocator ska_mem_get_global_allocator() {
  if (!isAllocatorValid(&current_alloc)) {
    current_alloc = standard_allocator;
  }
  return current_alloc;
}

void ska_mem_reset_to_default_global_allocator() {
  current_alloc = standard_allocator;
}

SkaAllocator ska_mem_standard_allocator() { return standard_allocator; }

void* ska_mem_global_allocate(usize bytes) {
  if (!isAllocatorValid(&current_alloc)) {
    current_alloc = standard_allocator;
  }
  void *memory = current_alloc.allocate(&current_alloc, bytes);
  SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d",
                 bytes);
  return memory;
}

void* ska_mem_global_allocate_zeroed(usize bytes) {
  if (!isAllocatorValid(&current_alloc)) {
    current_alloc = standard_allocator;
  }
  void *memory = current_alloc.allocate_zeroed(&current_alloc, bytes);
  SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d",
                 bytes);
  return memory;
}

void* ska_mem_global_reallocate(void* memory, usize bytes) {
  if (!isAllocatorValid(&current_alloc)) {
    current_alloc = standard_allocator;
  }
  void *reallocated_memory =
      current_alloc.reallocate(&current_alloc, memory, bytes);
  SKA_ASSERT_FMT(reallocated_memory,
                 "Out of memory or realloc failed!, size = %d", bytes);
  return reallocated_memory;
}

void ska_mem_global_free(void* memory) {
  if (!isAllocatorValid(&current_alloc)) {
    current_alloc = standard_allocator;
  }
  current_alloc.free(&current_alloc, memory);
}
