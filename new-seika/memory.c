#include "new-seika/memory.h"
#include "new-seika/assert.h"

void* ska_mem_allocate(size_t size) {
    void* memory = calloc(1, size);
    SKA_ASSERT_FMT(memory, "Out of memory or allocate failed!, size = %d", size);
    return memory;
}

void* ska_mem_allocate_c(size_t blocks, size_t size) {
    void* memory = calloc(blocks, size);
    SKA_ASSERT_FMT(memory, "Out of memory or allocate_c failed!, size = %d", size);
    return memory;
}

void* ska_mem_reallocate(void* memory, size_t size) {
    void* reallocatedMemory = realloc(memory, size);
    SKA_ASSERT_FMT(reallocatedMemory, "Out of memory or realloc failed!, size = %d", size);
    return reallocatedMemory;
}

void ska_mem_free(void* memory) {
    free(memory);
    memory = NULL;
}
