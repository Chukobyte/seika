#include "tick.h"

#include "memory.h"
#include "time.h"

SkaTick* ska_tick_create(SkaTickParams params) {
    SkaTick* tick = SKA_ALLOC_ZEROED(SkaTick);
    tick->updateFunc = params.updateFunc;
    tick->fixedUpdateFunc = params.fixedUpdateFunc;
    tick->updateInterval = 1000 / params.targetFPS;
    tick->fixedUpdateInterval = 1.0f / (f32)params.targetFixedFPS;
    tick->fixedDeltaTime = tick->fixedUpdateInterval;
    return tick;
}

void ska_tick_destroy(SkaTick* tick) {
    SKA_FREE(tick);
}

void ska_tick_update(SkaTick* tick) {
    const uint64 newTime = ska_timer_get_global_time();
    const uint64 deltaTime = newTime - tick->currentTime;
    const f32 deltaTimeSeconds = (f32)deltaTime / 1000.f;

    tick->currentTime = newTime;
    if (tick->updateFunc) {
        tick->updateFunc(deltaTimeSeconds);
    }
    if (tick->fixedUpdateFunc) {
        tick->accumulator += deltaTimeSeconds;
        while (tick->accumulator >= tick->fixedUpdateInterval) {
            tick->fixedUpdateFunc(tick->fixedDeltaTime);
            tick->accumulator -= tick->fixedUpdateInterval;
        }
    }
}
