#include "tick.h"

#include "memory.h"
#include "time.h"

SkaTick* ska_tick_create(SkaTickParams params) {
    SkaTick* tick = SKA_ALLOC_ZEROED(SkaTick);
    tick->update_func = params.update_func;
    tick->fixed_update_func = params.fixed_update_func;
    tick->update_interval = 1000 / params.target_fps;
    tick->fixed_update_interval = 1.0f / (f32)params.target_fixed_fps;
    tick->fixed_delta_time = tick->fixed_update_interval;
    return tick;
}

void ska_tick_destroy(SkaTick* tick) {
    SKA_FREE(tick);
}

// Caps a stalled frame (app backgrounded, window dragged, debugger) so fixed updates don't spiral to catch up
#define SKA_TICK_MAX_DELTA_SECONDS 0.25f

void ska_tick_update(SkaTick* tick) {
    const uint64 new_time = ska_timer_get_global_time();
    const uint64 delta_time = new_time - tick->current_time;
    f32 delta_time_seconds = (f32)delta_time / 1000.f;
    if (delta_time_seconds > SKA_TICK_MAX_DELTA_SECONDS) {
        delta_time_seconds = SKA_TICK_MAX_DELTA_SECONDS;
    }

    tick->current_time = new_time;
    if (tick->update_func) {
      tick->update_func(delta_time_seconds);
    }
    if (tick->fixed_update_func) {
      tick->accumulator += delta_time_seconds;
      while (tick->accumulator >= tick->fixed_update_interval) {
        tick->fixed_update_func(tick->fixed_delta_time);
        tick->accumulator -= tick->fixed_update_interval;
      }
    }
}
