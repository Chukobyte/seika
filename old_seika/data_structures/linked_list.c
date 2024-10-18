#include "linked_list.h"

#include <string.h>
#include <stdbool.h>

#include "seika/memory.h"
#include "seika/assert.h"

static inline void linked_list_delete_all_nodes(SkaLinkedList* list);
static inline SkaLinkedListNode* linked_list_create_node(SkaLinkedList* list, void* value, SkaLinkedListNode* next);
static inline void* linked_list_duplicate_node_value(SkaLinkedList* list, SkaLinkedListNode* node);

// Public functions
SkaLinkedList* ska_linked_list_create(usize valueSize) {
    SkaLinkedList* list = SKA_MEM_ALLOCATE(SkaLinkedList);
    list->valueSize = valueSize;
    return list;
}

void ska_linked_list_destroy(SkaLinkedList* list) {
    linked_list_delete_all_nodes(list);
    SKA_MEM_FREE(list);
}

void ska_linked_list_append(SkaLinkedList* list, void* value) {
    SKA_ASSERT(list != NULL);
    SKA_ASSERT(value != NULL);
    SkaLinkedListNode* prevTail = list->tail;
    list->tail = linked_list_create_node(list, value, NULL);
    if (prevTail != NULL) {
        prevTail->next = list->tail;
    }
    // If no head, set the head to tail.
    if (list->head == NULL) {
        list->head = list->tail;
    }
    list->size++;
}

void ska_linked_list_prepend(SkaLinkedList* list, void* value) {
    SKA_ASSERT(list != NULL);
    SKA_ASSERT(value != NULL);
    list->head = linked_list_create_node(list, value, list->head);
    // If no tail, set the tail to head.
    if (list->tail == NULL) {
        list->tail = list->head;
    }
    list->size++;
}

void ska_linked_list_insert(SkaLinkedList* list, void* value, usize index) {
    SKA_ASSERT(list != NULL);
    SKA_ASSERT(value != NULL);
    // Check if at beginning or end of list, then just call those functions if so...
    if (index > list->size) {
        // Index is out of bounds, can return error if needed...
        return;
    } else if (index == 0) {
        ska_linked_list_prepend(list, value);
        return;
    } else if (index == list->size) {
        ska_linked_list_append(list, value);
        return;
    }
    // Now we know it's at the middle of the list, iterate through nodes
    usize nodeIndex = 1;
    SkaLinkedListNode* prevNode = list->head;
    SkaLinkedListNode* currentIndexNode = list->head->next; // Get the next value since the head exists and was checked
    while (currentIndexNode != NULL) {
        // We found the index node, now insert
        if (index == nodeIndex) {
            SkaLinkedListNode* currentIndexNodeNext = currentIndexNode->next;
            prevNode->next = linked_list_create_node(list, value, currentIndexNodeNext);
            list->size++;
            return;
        }
        prevNode = currentIndexNode;
        currentIndexNode = currentIndexNode->next;
        nodeIndex++;
    }
    SKA_ASSERT_FMT(false, "Error with inserting into array list!");
}

void* ska_linked_list_get(SkaLinkedList* list, usize index) {
    if (index < list->size) {
        usize nodeIndex = 0;
        SkaLinkedListNode* node = list->head;
        while (node != NULL) {
            if (index == nodeIndex) {
                return node->value;
            }
            node = node->next;
            nodeIndex++;
        }
    }
    return NULL;
}

void* ska_linked_list_get_front(SkaLinkedList* list) {
    return list->head->value;
}

void* ska_linked_list_get_back(SkaLinkedList* list) {
    return list->tail->value;
}

void* ska_linked_list_pop(SkaLinkedList* list, usize index) {
    if (list->size > 0 && index < list->size) {
        if (index == 0) {
            return ska_linked_list_pop_front(list);
        } else if (index == list->size - 1) {
            return ska_linked_list_pop_back(list);
        }

        // Search for node
        usize nodeIndex = 1;
        SkaLinkedListNode* nodeBeforePopped = list->head->next;
        while (nodeBeforePopped != NULL) {
            if (index - 1 == nodeIndex) {
                break;
            }
            nodeBeforePopped = nodeBeforePopped->next;
            nodeIndex++;
        }
        // Connect the previous node with the node that's being popped's next node
        SKA_ASSERT(nodeBeforePopped->next);
        SkaLinkedListNode* nodeToPop = nodeBeforePopped->next;
        nodeBeforePopped->next = nodeToPop->next;
        list->size--;
        void* poppedValue = linked_list_duplicate_node_value(list, nodeToPop);
        SKA_MEM_FREE(nodeToPop->value);
        SKA_MEM_FREE(nodeToPop);
        return poppedValue;
    }
    return NULL;
}

void* ska_linked_list_pop_front(SkaLinkedList* list) {
    if (list->size > 0) {
        SkaLinkedListNode* nodeToPop = list->head;
        list->head = nodeToPop->next;
        list->size--;
        if (list->size <= 1) {
            list->tail = list->head;
        }
        void* poppedValue = linked_list_duplicate_node_value(list, nodeToPop);
        SKA_MEM_FREE(nodeToPop->value);
        SKA_MEM_FREE(nodeToPop);
        return poppedValue;
    }
    return NULL;
}

void* ska_linked_list_pop_back(SkaLinkedList* list) {
    if (list->size == 1) {
        return ska_linked_list_pop_front(list);
    } else if (list->size == 2) {
        SkaLinkedListNode* nodeToPop = list->tail;
        list->tail = list->head;
        list->size--;
        void* poppedValue = linked_list_duplicate_node_value(list, nodeToPop);
        SKA_MEM_FREE(nodeToPop);
        return poppedValue;
    } else if (list->size >= 3) {
        SkaLinkedListNode* nodeToPop = list->head->next;
        SkaLinkedListNode* nodePrev = list->head;
        while (true) {
            if (nodeToPop->next == NULL) {
                break;
            }
            nodePrev = nodePrev->next;
            nodeToPop = nodeToPop->next;
        }
        // Now that we have node to pop and previous node, pop it
        nodePrev->next = nodeToPop->next;
        list->size--;
        void* poppedValue = linked_list_duplicate_node_value(list, nodeToPop);
        SKA_MEM_FREE(nodeToPop->value);
        SKA_MEM_FREE(nodeToPop);
        return poppedValue;
    }
    return NULL;
}

bool ska_linked_list_remove(SkaLinkedList* list, void* value) {
    SkaLinkedListNode* currentNode = list->head;
    SkaLinkedListNode* prevNode = NULL;
    while (currentNode != NULL) {
        if (memcmp(currentNode->value, value, list->valueSize) == 0) {
            SkaLinkedListNode* nodeToDelete = currentNode;
            // First entry
            if (prevNode == NULL) {
                list->head = currentNode->next;
                if (list->size - 1 == 1) {
                    list->tail = list->head;
                }
            } else {
                prevNode->next = currentNode->next;
                // If at end, set tail
                if (prevNode->next == NULL) {
                    list->tail = prevNode;
                }
            }

            list->size--;
            SKA_MEM_FREE(nodeToDelete->value);
            SKA_MEM_FREE(nodeToDelete);
            return true;
        }
        prevNode = currentNode;
        currentNode = currentNode->next;
    }
    return false;
}

bool ska_linked_list_is_empty(SkaLinkedList* list) {
    return list->size == 0;
}

void ska_linked_list_sort(SkaLinkedList* list, SkaLinkedListIsGreaterThanFunc isGreaterThanFunc) {
    if (list->head == NULL) {
        return;
    }
    for (SkaLinkedListNode* current = list->head; current->next != NULL; current = current->next) {
        for (SkaLinkedListNode* next = current->next; next != NULL; next = next->next) {
            if (isGreaterThanFunc(current->value, next->value)) {
                void* temp = current->value;
                current->value = next->value;
                next->value = temp;
            }
        }
    }
}

void ska_linked_list_clear(SkaLinkedList* list) {
    linked_list_delete_all_nodes(list);
}

// Private functions
static SkaLinkedListNode* linked_list_create_node(SkaLinkedList* list, void* value, SkaLinkedListNode* next) {
    SkaLinkedListNode* node = SKA_MEM_ALLOCATE(SkaLinkedListNode);
    node->value = SKA_MEM_ALLOCATE_SIZE(list->valueSize);
    memcpy(node->value, value, list->valueSize);
    node->next = next;
    return node;
}

static void* linked_list_duplicate_node_value(SkaLinkedList* list, SkaLinkedListNode* node) {
    void* dupValue = SKA_MEM_ALLOCATE_SIZE(list->valueSize);
    memcpy(dupValue, node->value, list->valueSize);
    return dupValue;
}

static void linked_list_delete_all_nodes(SkaLinkedList* list) {
    SkaLinkedListNode* nodeToDelete = list->head;
    while (nodeToDelete != NULL) {
        SkaLinkedListNode* nextNode = nodeToDelete->next;
        SKA_MEM_FREE(nodeToDelete->value);
        SKA_MEM_FREE(nodeToDelete);
        nodeToDelete = nextNode;
    }
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
}
