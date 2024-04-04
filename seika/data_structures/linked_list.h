#pragma once

#include "seika/defines.h"

#define SKA_LINKED_LIST_FOR_EACH(list, element) for((element) = (list)->head; (element) != NULL; (element) = (element)->next)

// Should return whether the array list node is greater than the other
typedef bool (*SkaLinkedListIsGreaterThanFunc) (void*, void*);

// Seika implementation of a single linked list
typedef struct SkaLinkedListNode {
    struct SkaLinkedListNode* next;
    void* value;
} SkaLinkedListNode;

// Generic array list, use 'ska_linked_list_create' to allocate on the heap
typedef struct SkaLinkedList {
    usize valueSize;
    usize size;
    SkaLinkedListNode* head;
    SkaLinkedListNode* tail;
} SkaLinkedList;

// Will create a new array list with all it's element values set to the passed in 'valueSize'
SkaLinkedList* ska_linked_list_create(usize valueSize);
// Will complete destroy the array list freeing up all its memory
void ska_linked_list_destroy(SkaLinkedList* list);

// Adds an item at the end of the list
void ska_linked_list_append(SkaLinkedList* list, void* value);
// Adds an item at the beginning of the list
void ska_linked_list_prepend(SkaLinkedList* list, void* value);
// Inserts an item at the specified index
void ska_linked_list_insert(SkaLinkedList* list, void* value, usize index);

// Returns an item from the list from specified index
void* ska_linked_list_get(SkaLinkedList* list, usize index);
// Same as 'ska_linked_list_get' but returns item from index 0
void* ska_linked_list_get_front(SkaLinkedList* list);
// Same as 'ska_linked_list_get' but returns item from index (size - 1)
void* ska_linked_list_get_back(SkaLinkedList* list);

// Removes and item and returns from the list from specified index, caller needs to manage memory
void* ska_linked_list_pop(SkaLinkedList* list, usize index);
// Same as 'ska_linked_list_pop' but removes and item from index 0, caller needs to manage memory
void* ska_linked_list_pop_front(SkaLinkedList* list);
// Same as 'ska_linked_list_pop' but removes and item from index (size - 1), caller needs to manage memory
void* ska_linked_list_pop_back(SkaLinkedList* list);
// Removes the first item from the list who is equal to the passed in value
bool ska_linked_list_remove(SkaLinkedList* list, void* value);

// Returns true if the array list size == 0
bool ska_linked_list_is_empty(SkaLinkedList* list);
// Will sort the array list, using the passed in 'isGreaterThanFunc' as a comparator for sorting
void ska_linked_list_sort(SkaLinkedList* list, SkaLinkedListIsGreaterThanFunc isGreaterThanFunc);
// Will remove all items from the array list
void ska_linked_list_clear(SkaLinkedList* list);
