#include "event.h"

#include "memory.h"
#include "seika/assert.h"
#include "data_structures/array_utils.h"

//--- Observer ---//
SkaObserver* ska_observer_new(SkaObserverOnNotify on_notify_func) {
    SKA_ASSERT(on_notify_func != NULL);
    SkaObserver* observer = SKA_ALLOC_ZEROED(SkaObserver);
    observer->on_notify = on_notify_func;
    return observer;
}

void ska_observer_delete(SkaObserver* observer) {
    SKA_FREE(observer);
}

//--- Event ---//
SkaEvent* ska_event_new() {
    SkaEvent* event = SKA_ALLOC_ZEROED(SkaEvent);
    event->observer_count = 0;
    return event;
}

void ska_event_delete(SkaEvent* event) {
    SKA_FREE(event);
}

bool ska_event_register_observer(SkaEvent* event, SkaObserver* observer) {
    SKA_ASSERT(event != NULL);
    SKA_ASSERT(observer != NULL);
    SKA_ASSERT_FMT(event->observer_count + 1 < SKA_MAX_OBSERVERS, "Reached max observer count, consider increasing 'SKA_MAX_OBSERVERS'!");
    event->observers[event->observer_count++] = observer;
    return true;
}

bool ska_event_unregister_observer(SkaEvent* event, SkaObserver* observer) {
    SKA_ARRAY_UTILS_REMOVE_ARRAY_ITEM(event->observers, event->observer_count, observer, NULL);
    return true;
}

void ska_event_notify_observers(SkaEvent* event, SkaSubjectNotifyPayload* payload) {
    for (usize i = 0; i < event->observer_count; i++) {
        event->observers[i]->on_notify(payload);
    }
}
