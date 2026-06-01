#pragma once

#include "seika/defines.h"
#include "seika/memory.h"

#define SKA_ARRAY_LIST_GET(LIST, TYPE, INDEX) (TYPE)ska_array_list_get((LIST), (INDEX))
#define SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE) ((TYPE)((LIST)->data))
#define SKA_ARRAY_LIST_FOR_EACH(LIST, TYPE, VALUE) \
for(TYPE VALUE = SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE); VALUE < (TYPE)((char*)SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE) + (LIST)->size * (LIST)->valueSize); VALUE++)
#define SKA_ARRAY_LIST_TO_ARRAY(LIST, TYPE) ((TYPE*)((LIST)->data))

typedef bool (*SkaArrayListCmp)(const void*, const void*);

// Generic array list, use 'ska_array_list_create' to allocate on the heap
typedef struct SkaArrayList {
    SkaAllocator allocator;
    void* data;
    usize valueSize; // Size of each element of the data
    usize size; // How many elements are in the array
    usize capacity; // Max elements allowed in array before resizing
    usize initialCapacity; // Min capacity for the array list
} SkaArrayList;

// Will create a new array list with all it's element values set to the passed in 'valueSize'
SkaArrayList ska_array_list_init_default_capacity(usize valueSize);
SkaArrayList ska_array_list_init(usize valueSize, usize initialCapacity);
SkaArrayList ska_array_list_init_a(SkaAllocator allocator, usize valueSize, usize initialCapacity);
// Will complete destroy the array list freeing up all its memory
void ska_array_list_deinit(SkaArrayList* list);
// Inserts an item at the end of the list
bool ska_array_list_append(SkaArrayList* list, const void* value);
// Returns an item from the list from specified index
void* ska_array_list_get(SkaArrayList* list, usize index);
const void* ska_array_list_get_const(const SkaArrayList* list, usize index);
// Removes the first item from the list who is equal to the passed in value
bool ska_array_list_remove(SkaArrayList* list, const void* value);
bool ska_array_list_remove2(SkaArrayList* list, const void* value, SkaArrayListCmp compareFunc);
bool ska_array_list_remove_by_index(SkaArrayList* list, usize index);
// Returns true if the item exists within the list
bool ska_array_list_has(const SkaArrayList* list, const void* value);
bool ska_array_list_has2(const SkaArrayList* list, const void* value, SkaArrayListCmp compareFunc);
// Returns true if the array list size == 0
bool ska_array_list_is_empty(const SkaArrayList *list);
// Will remove all items from the array list
void ska_array_list_clear(SkaArrayList *list);
// Will allocate memory to match the capacity, if the current capacity is less that 'newCapacity'
void ska_array_list_reserve(SkaArrayList* list, usize newCapacity);
