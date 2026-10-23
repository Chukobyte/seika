#pragma once

#include "seika/assert.h"
#include "seika/memory.h"

// Forward declare the macros
#define SKA_EVENT_DECLARE(STRUCT_NAME, PAYLOAD_TYPE) \
    SKA_EVENT_DECLARE_INTERNAL(STRUCT_NAME, PAYLOAD_TYPE)

#define SKA_EVENT_IMPLEMENT(STRUCT_NAME, PAYLOAD_TYPE) \
    SKA_EVENT_IMPLEMENT_INTERNAL(STRUCT_NAME, PAYLOAD_TYPE)

// Internal declaration macro
#define SKA_EVENT_DECLARE_INTERNAL(STRUCT_NAME, PAYLOAD_TYPE) \
    typedef void (*STRUCT_NAME##Callback)(PAYLOAD_TYPE payload); \
    \
    typedef struct STRUCT_NAME##CallbackNode { \
        struct STRUCT_NAME##CallbackNode* next; \
        STRUCT_NAME##Callback callback; \
    } STRUCT_NAME##CallbackNode; \
    \
    typedef struct STRUCT_NAME { \
        SkaAllocator allocator; \
        usize subscriber_count; \
        STRUCT_NAME##CallbackNode* callbacks; \
    } STRUCT_NAME; \
    \
    STRUCT_NAME STRUCT_NAME##_create(void); \
    STRUCT_NAME STRUCT_NAME##_create_a(SkaAllocator allocator); \
    void STRUCT_NAME##_destroy(STRUCT_NAME* event); \
    void STRUCT_NAME##_subscribe(STRUCT_NAME* event, STRUCT_NAME##Callback callback); \
    bool STRUCT_NAME##_unsubscribe(STRUCT_NAME* event, STRUCT_NAME##Callback callback); \
    void STRUCT_NAME##_notify(STRUCT_NAME* event, PAYLOAD_TYPE payload); \
    usize STRUCT_NAME##_subscriber_count(const STRUCT_NAME* event); \
    bool STRUCT_NAME##_has_subscribers(const STRUCT_NAME* event); \
    void STRUCT_NAME##_clear(STRUCT_NAME* event);

// Internal implementation macro
#define SKA_EVENT_IMPLEMENT_INTERNAL(STRUCT_NAME, PAYLOAD_TYPE) \
    STRUCT_NAME STRUCT_NAME##_create(void) { \
        return STRUCT_NAME##_create_a(ska_mem_get_global_allocator()); \
    } \
    \
    STRUCT_NAME STRUCT_NAME##_create_a(SkaAllocator allocator) { \
        return (STRUCT_NAME){ \
            .allocator = allocator, \
            .subscriber_count = 0, \
            .callbacks = NULL \
        }; \
    } \
    \
    void STRUCT_NAME##_destroy(STRUCT_NAME* event) { \
        STRUCT_NAME##CallbackNode* node = event->callbacks; \
        while (node != NULL) { \
            STRUCT_NAME##CallbackNode* next = node->next; \
            SKA_FREE_A(event->allocator, node); \
            node = next; \
        } \
        event->callbacks = NULL; \
        event->subscriber_count = 0; \
    } \
    \
    void STRUCT_NAME##_subscribe(STRUCT_NAME* event, STRUCT_NAME##Callback callback) { \
        SKA_ASSERT(event != NULL); \
        SKA_ASSERT(callback != NULL); \
        \
        STRUCT_NAME##CallbackNode* node = SKA_ALLOC_A(event->allocator, STRUCT_NAME##CallbackNode); \
        node->callback = callback; \
        node->next = event->callbacks; \
        event->callbacks = node; \
        event->subscriber_count++; \
    } \
    \
    bool STRUCT_NAME##_unsubscribe(STRUCT_NAME* event, STRUCT_NAME##Callback callback) { \
        SKA_ASSERT(event != NULL); \
        SKA_ASSERT(callback != NULL); \
        \
        STRUCT_NAME##CallbackNode* node = event->callbacks; \
        STRUCT_NAME##CallbackNode* prev = NULL; \
        \
        while (node != NULL) { \
            if (node->callback == callback) { \
                if (prev != NULL) { \
                    prev->next = node->next; \
                } else { \
                    event->callbacks = node->next; \
                } \
                SKA_FREE_A(event->allocator, node); \
                event->subscriber_count--; \
                return true; \
            } \
            prev = node; \
            node = node->next; \
        } \
        return false; \
    } \
    \
    void STRUCT_NAME##_notify(STRUCT_NAME* event, PAYLOAD_TYPE payload) { \
        SKA_ASSERT(event != NULL); \
        \
        STRUCT_NAME##CallbackNode* node = event->callbacks; \
        while (node != NULL) { \
            node->callback(payload); \
            node = node->next; \
        } \
    } \
    \
    usize STRUCT_NAME##_subscriber_count(const STRUCT_NAME* event) { \
        SKA_ASSERT(event != NULL); \
        return event->subscriber_count; \
    } \
    \
    bool STRUCT_NAME##_has_subscribers(const STRUCT_NAME* event) { \
        SKA_ASSERT(event != NULL); \
        return event->subscriber_count > 0; \
    } \
    \
    void STRUCT_NAME##_clear(STRUCT_NAME* event) { \
        SKA_ASSERT(event != NULL); \
        STRUCT_NAME##_destroy(event); \
        event->callbacks = NULL; \
        event->subscriber_count = 0; \
    }
