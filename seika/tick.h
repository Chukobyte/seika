#pragma once

#include "defines.h"

typedef void (*SkaTickUpdateFunc) (f32);
typedef void (*SkaTickFixedUpdateFunc) (f32);

typedef struct SkaTick {
  SkaTickUpdateFunc update_func;
  SkaTickFixedUpdateFunc fixed_update_func;
  uint64 current_time;
  f32 accumulator;
  uint32 update_interval;
  f32 fixed_update_interval;
  f32 fixed_delta_time;
} SkaTick;

typedef struct SkaTickParams {
  uint32 target_fps;
  uint32 target_fixed_fps;
  SkaTickUpdateFunc update_func;
  SkaTickFixedUpdateFunc fixed_update_func;
} SkaTickParams;

SkaTick* ska_tick_create(SkaTickParams params);
void ska_tick_destroy(SkaTick* tick);
void ska_tick_update(SkaTick* tick);
