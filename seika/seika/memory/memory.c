#include "memory.h"
#include "seika/assert.h"

#pragma region
// Internal standard allocator implementation

typedef struct Allocation {
    void* ptr;
    usize bytes;
    struct Allocation* next;
} Allocation;

static Allocation* allocationList = NULL;

static void track_allocation(void* memory, usize bytes) {
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

// static void free_all_allocations(bool freeLeakedMemory) {
//     Allocation* current = allocationList;
//     while (current) {
//         Allocation* next = current->next;
//         if (freeLeakedMemory) {
//             free(current->ptr);
//         }
//         free(current);
//         current = next;
//     }
//     allocationList = NULL;
// }

static bool isAllocatorValid(const SkaAllocator* allocator) {
    return allocator->allocate
        && allocator->allocate_zeroed
        && allocator->reallocate
        && allocator->free;
}

static void* ska_mem_standard_allocator_allocate(SkaAllocator* allocator, usize bytes) {
    void* memory = malloc(bytes);
    SKA_ASSERT_REL_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    track_allocation(memory, bytes);
    return memory;
}

static void* ska_mem_standard_allocator_allocate_zeroed(SkaAllocator* allocator, usize bytes) {
    void* memory = calloc(1, bytes);
    SKA_ASSERT_REL_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    track_allocation(memory, bytes);
    return memory;
}

static void* ska_mem_standard_allocator_reallocate(SkaAllocator* allocator, void* memory, usize bytes) {
    untrack_allocation(memory);
    void* reallocatedMemory = realloc(memory, bytes);
    SKA_ASSERT_REL_FMT(reallocatedMemory, "Out of memory or realloc failed!, size = %d", bytes);
    track_allocation(reallocatedMemory, bytes);
    return reallocatedMemory;
}

static void ska_mem_standard_allocator_free(SkaAllocator* allocator, void* memory) {
    untrack_allocation(memory);
    free(memory);
}

// static void ska_mem_standard_allocator_free_all() {
//     free_all_allocations(true);
// }
//
// static bool ska_mem_standard_allocator_report_leaks() {
//     // If there is an allocation list there is a memory leak
//     return allocationList != NULL;
// }

// End standard memory allocation
#pragma endregion

static const SkaAllocator standardAllocator = {
    .context = NULL,
    .allocate = ska_mem_standard_allocator_allocate,
    .allocate_zeroed = ska_mem_standard_allocator_allocate_zeroed,
    .reallocate = ska_mem_standard_allocator_reallocate,
    .free = ska_mem_standard_allocator_free,
};

static SkaAllocator currentAlloc = {0};

void ska_mem_set_global_allocator(const SkaAllocator allocator) {
    SKA_ASSERT_FMT(isAllocatorValid(&allocator), "Must implement all allocator functions before setting");
    currentAlloc = allocator;
}

SkaAllocator ska_mem_get_global_allocator() {
    if (!isAllocatorValid(&currentAlloc)) {
        currentAlloc = standardAllocator;
    }
    return currentAlloc;
}

void ska_reset_to_default_allocator() {
    currentAlloc = standardAllocator;
}

SkaAllocator ska_mem_standard_allocator() {
    return standardAllocator;
}

void* ska_mem_global_allocate(usize bytes) {
    if (!isAllocatorValid(&currentAlloc)) {
        currentAlloc = standardAllocator;
    }
    void* memory = currentAlloc.allocate(&currentAlloc, bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

void* ska_mem_global_allocate_zeroed(usize bytes) {
    if (!isAllocatorValid(&currentAlloc)) {
        currentAlloc = standardAllocator;
    }
    void* memory = currentAlloc.allocate_zeroed(&currentAlloc, bytes);
    SKA_ASSERT_FMT(memory, "Out of memory or allocation failed!, size = %d", bytes);
    return memory;
}

void* ska_mem_global_reallocate(void* memory, usize bytes) {
    if (!isAllocatorValid(&currentAlloc)) {
        currentAlloc = standardAllocator;
    }
    void* reallocatedMemory = currentAlloc.reallocate(&currentAlloc, memory, bytes);
    SKA_ASSERT_FMT(reallocatedMemory, "Out of memory or realloc failed!, size = %d", bytes);
    return reallocatedMemory;
}

void ska_mem_global_free(void* memory) {
    if (!isAllocatorValid(&currentAlloc)) {
        currentAlloc = standardAllocator;
    }
    currentAlloc.free(&currentAlloc, memory);
}

// void ska_mem_global_free_all() {
//     currentAlloc.free_all();
// }
//
// bool ska_mem_global_report_leaks() {
//     return currentAlloc.report_leaks();
// }
