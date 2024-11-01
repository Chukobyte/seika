#include "seika/memory.h"
#include "seika/assert.h"

// Internal default allocator implementation

typedef struct Allocation {
    const void* ptr;
    usize bytes;
    struct Allocation* next;
} Allocation;

static Allocation* allocationList = NULL;

static void track_allocation(const void* memory, usize bytes) {
    Allocation* newAlloc = (Allocation*)malloc(sizeof(Allocation));
    SKA_ASSERT(newAlloc);
    newAlloc->ptr = memory;
    newAlloc->bytes = bytes;
    newAlloc->next = allocationList;
    allocationList = newAlloc;
}

static void untrack_allocation(const void* memory) {
    Allocation* current = allocationList;
    Allocation* prev = NULL;
    while (current) {
        if (current->ptr == memory) {
            if (prev) {
                prev->next = current->next;
            } else {
                allocationList = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

static bool isAllocatorValid(const SkaMemAllocator* allocator) {
    return allocator->allocate
        && allocator->allocate_zeroed
        && allocator->reallocate
        && allocator->free
        && allocator->report_leaks;
}

static void* internal_mem_allocate(usize bytes) {
    void* memory = malloc(bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    track_allocation(memory, bytes);
    return memory;
}

static void* internal_mem_allocate_zeroed(usize bytes) {
    void* memory = calloc(1, bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    track_allocation(memory, bytes);
    return memory;
}

static void* internal_mem_reallocate(void* memory, usize bytes) {
    untrack_allocation(memory);
    void* reallocatedMemory = realloc(memory, bytes);
    SKA_ASSERT_FMT(reallocatedMemory, "Out of memory or realloc failed!, size = %d", bytes);
    track_allocation(reallocatedMemory, bytes);
    return reallocatedMemory;
}

static void internal_mem_free(void* memory) {
    untrack_allocation(memory);
    free(memory);
    memory = NULL;
}

static bool internal_mem_report_leaks() {
    // If there is an allocation list there is a memory leak
    return allocationList != NULL;
}

static const SkaMemAllocator defaultAlloc = {
    .allocate = internal_mem_allocate,
    .allocate_zeroed = internal_mem_allocate_zeroed,
    .reallocate = internal_mem_reallocate,
    .free = internal_mem_free,
    .report_leaks = internal_mem_report_leaks
};

static SkaMemAllocator currentAlloc = {
    .allocate = internal_mem_allocate,
    .allocate_zeroed = internal_mem_allocate_zeroed,
    .reallocate = internal_mem_reallocate,
    .free = internal_mem_free,
    .report_leaks = internal_mem_report_leaks
};


void ska_set_current_allocator(const SkaMemAllocator allocator) {
    SKA_ASSERT_FMT(isAllocatorValid(&allocator), "Must implement all allocator functions before setting");
    currentAlloc = allocator;
}

const SkaMemAllocator* ska_mem_get_current_allocator() {
    return &currentAlloc;
}

void ska_reset_to_default_allocator() {
    currentAlloc = defaultAlloc;
}

void* ska_mem_allocate(usize bytes) {
    void* memory = currentAlloc.allocate(bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

void* ska_mem_allocate_zeroed(usize bytes) {
    void* memory = currentAlloc.allocate_zeroed(bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

void* ska_mem_reallocate(void* memory, usize bytes) {
    void* reallocatedMemory = currentAlloc.reallocate(memory, bytes);
    SKA_ASSERT_FMT(reallocatedMemory, "Out of memory or realloc failed!, size = %d", bytes);
    return reallocatedMemory;
}

void ska_mem_free(void* memory) {
    currentAlloc.free(memory);
}

bool ska_mem_report_leaks() {
    return currentAlloc.report_leaks();
}
