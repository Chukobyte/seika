#pragma once

#include "defines.h"

typedef void (*SkaTickUpdateFunc) (f32);
typedef void (*SkaTickFixedUpdateFunc) (f32);

typedef struct SkaTick {
    SkaTickUpdateFunc updateFunc;
    SkaTickFixedUpdateFunc fixedUpdateFunc;
    uint64 currentTime;
    f32 accumulator;
    uint32 updateInterval;
    f32 fixedUpdateInterval;
    f32 fixedDeltaTime;
} SkaTick;

typedef struct SkaTickParams {
    uint32 targetFPS;
    uint32 targetFixedFPS;
    SkaTickUpdateFunc updateFunc;
    SkaTickFixedUpdateFunc fixedUpdateFunc;
} SkaTickParams;

SkaTick* ska_tick_create(SkaTickParams params);
void ska_tick_destroy(SkaTick* tick);
void ska_tick_update(SkaTick* tick);
