#pragma once

// TODO: Figure out if this should move to a different place

#include "defines.h"

#if defined(PLATFORM_WINDOWS)
#  include <windows.h>
#elif defined(PLATFORM_EMSCRIPTEN)
#  include <emscripten/emscripten.h>
#else
#  include <time.h>
#  include <errno.h>
#  include <sys/time.h>
#endif

typedef struct SkaTimer {
#if defined(PLATFORM_WINDOWS)
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
#elif defined(PLATFORM_EMSCRIPTEN)
    double start = 0.0;
#else
    // POSIX: Use clock_gettime with CLOCK_MONOTONIC
    struct timespec start;
#endif

    bool hasStarted;
} SkaTimer;

// Will make the CPU sleep for the length of the milliseconds param
void ska_delay(uint32 milliseconds);

// SkaTimer

void ska_timer_start(SkaTimer* timer);
void ska_timer_stop(SkaTimer* timer);
uint64 ska_timer_get_time(const SkaTimer* timer);
// Used to keep track of ticks since the program has started, should call to initialize
uint64 ska_timer_get_global_time();
