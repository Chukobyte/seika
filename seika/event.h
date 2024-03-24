#pragma once

#include <stddef.h>
#include <stdbool.h>

#define SKA_MAX_OBSERVERS 8

typedef struct SkaSubjectNotifyPayload {
    void* data; // Primary data, be sure to cast properly
} SkaSubjectNotifyPayload;

typedef void (*SkaObserverOnNotify)(SkaSubjectNotifyPayload*);

// An observer that can subscribe to a subject
typedef struct SkaObserver {
    SkaObserverOnNotify on_notify;
} SkaObserver;

SkaObserver* ska_observer_new(SkaObserverOnNotify onNotifyFunc);
void ska_observer_delete(SkaObserver* observer);

// A subscribable event
typedef struct SkaEvent {
    size_t observerCount;
    SkaObserver* observers[SKA_MAX_OBSERVERS];
} SkaEvent;

SkaEvent* ska_event_new();
void ska_event_delete(SkaEvent* event);
bool ska_event_register_observer(SkaEvent* event, SkaObserver* observer);
bool ska_event_unregister_observer(SkaEvent* event, SkaObserver* observer);
void ska_event_notify_observers(SkaEvent* event, SkaSubjectNotifyPayload* payload);
