#include "event.h"

#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/data_structures/array_utils.h"

//--- Observer ---//
SkaObserver* ska_observer_new(SkaObserverOnNotify onNotifyFunc) {
    SKA_ASSERT(onNotifyFunc != NULL);
    SkaObserver* observer = SKA_MEM_ALLOCATE(SkaObserver);
    observer->on_notify = onNotifyFunc;
    return observer;
}

void ska_observer_delete(SkaObserver* observer) {
    SKA_MEM_FREE(observer);
}

//--- Event ---//
SkaEvent* ska_event_new() {
    SkaEvent* event = SKA_MEM_ALLOCATE(SkaEvent);
    event->observerCount = 0;
    return event;
}

void ska_event_delete(SkaEvent* event) {
    SKA_MEM_FREE(event);
}

bool ska_event_register_observer(SkaEvent* event, SkaObserver* observer) {
    SKA_ASSERT(event != NULL);
    SKA_ASSERT(observer != NULL);
    SKA_ASSERT_FMT(event->observerCount + 1 < SKA_MAX_OBSERVERS, "Reached max observer count, consider increasing 'SKA_MAX_OBSERVERS'!");
    event->observers[event->observerCount++] = observer;
    return true;
}

bool ska_event_unregister_observer(SkaEvent* event, SkaObserver* observer) {
    SKA_ARRAY_UTILS_REMOVE_ARRAY_ITEM(event->observers, event->observerCount, observer, NULL);
    return true;
}

void ska_event_notify_observers(SkaEvent* event, SkaSubjectNotifyPayload* payload) {
    for (usize i = 0; i < event->observerCount; i++) {
        event->observers[i]->on_notify(payload);
    }
}
