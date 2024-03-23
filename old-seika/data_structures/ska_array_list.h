#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "../memory/se_mem.h"

#define SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE) ((TYPE*)((LIST)->data))
#define SKA_ARRAY_LIST_FOR_EACH(LIST, TYPE, VALUE) for(TYPE* VALUE = SKA_ARRAY_LIST_GET_ARRAY(LIST, TYPE), *VALUE##_end = VALUE + (LIST)->size; VALUE < VALUE##_end; VALUE++)

// Generic array list, use 'ska_array_list_create' to allocate on the heap
typedef struct SkaArrayList {
    void* data;
    size_t valueSize; // Size of each element of the data
    size_t size; // How many elements are in the array
    size_t capacity; // Max elements allowed in array before resizing
    size_t initialCapacity; // Min capacity for the array list
} SkaArrayList;

// Will create a new array list with all it's element values set to the passed in 'valueSize'
SkaArrayList* ska_array_list_create_default_capacity(size_t valueSize);
SkaArrayList* ska_array_list_create(size_t valueSize, size_t initialCapacity);
// Will complete destroy the array list freeing up all its memory
void ska_array_list_destroy(SkaArrayList* list);
// Inserts an item at the end of the list
void ska_array_list_push_back(SkaArrayList* list, const void* value);
// Returns an item from the list from specified index
void* ska_array_list_get(SkaArrayList* list, size_t index);
// Removes the first item from the list who is equal to the passed in value
bool ska_array_list_remove(SkaArrayList* list, const void* value);
bool ska_array_list_remove_by_index(SkaArrayList* list, size_t index);
// Returns true if the array list size == 0
bool ska_array_list_is_empty(SkaArrayList *list);
// Will remove all items from the array list
void ska_array_list_clear(SkaArrayList *list);
