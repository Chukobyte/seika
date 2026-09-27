#include "time.h"

#if defined(PLATFORM_WINDOWS)
#  include <windows.h>
#elif defined(PLATFORM_EMSCRIPTEN)
#  include <emscripten/emscripten.h>
#else
#  include <time.h>
#  include <errno.h>
#  include <sys/time.h>
#endif

// TODO: We'll do some sort of init first

void ska_timer_start(SkaTimer* timer) {
  if (timer->has_started)
    return;
#if defined(PLATFORM_WINDOWS)
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start);
#elif defined(PLATFORM_EMSCRIPTEN)
    timer->start = emscripten_get_now();
#else
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
#endif
    timer->has_started = true;
}

// TODO: Implement
void ska_timer_stop(SkaTimer* timer) {
  if (!timer->has_started)
    return;
  timer->has_started = false;
}

// Get milliseconds since timer started
uint64 ska_timer_get_time(const SkaTimer* timer) {
#if defined(PLATFORM_WINDOWS)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    const uint64 elapsed = now.QuadPart - timer->start.QuadPart;
    return (elapsed * 1000) / timer->frequency.QuadPart;
#elif defined(PLATFORM_EMSCRIPTEN)
    return (uint64)(emscripten_get_now() - timer->start);
#else
    // POSIX: Use clock_gettime with CLOCK_MONOTONIC
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    const uint64 sec_diff = now.tv_sec - timer->start.tv_sec;
    const uint64 nsec_diff = now.tv_nsec - timer->start.tv_nsec;
    return sec_diff * 1000 + nsec_diff / 1000000;
#endif
}

uint64 ska_timer_get_global_time() {
  static SkaTimer global_timer = {0};
  ska_timer_start(&global_timer); // Initialize global timer if needed
  return ska_timer_get_time(&global_timer);
}

void ska_delay(uint32 milliseconds) {
#if defined(PLATFORM_WINDOWS)
    // Windows: Use Sleep (note: capital S)
    Sleep(milliseconds);

#elif defined(PLATFORM_EMSCRIPTEN)
    // Emscripten: Can't block, this is a no-op
    // Use emscripten_set_main_loop instead of manual delay
    (void)milliseconds;
#else
    // POSIX: Use nanosleep for better precision
    struct timespec req, rem;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000L;

    // Handle interruptions (EINTR)
    while (nanosleep(&req, &rem) == -1) {
        if (errno != EINTR) {
            break;
        }
        req = rem;
    }
#endif
}

// Alternative: Higher precision delay (microseconds)
void ska_delay_us(uint32 us) {
#if defined(PLATFORM_WINDOWS)
    // Windows high-precision delay using busy-wait for small delays
    if (us < 1000) {
        LARGE_INTEGER frequency, start, now;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);

        uint64 target_ticks = (us * frequency.QuadPart) / 1000000;
        do {
            QueryPerformanceCounter(&now);
        } while ((now.QuadPart - start.QuadPart) < (LONGLONG)target_ticks);
    } else {
        Sleep(us / 1000);
    }

#elif defined(PLATFORM_EMSCRIPTEN)
    (void)us;

#else
    struct timespec req, rem;
    req.tv_sec = us / 1000000;
    req.tv_nsec = (us % 1000000) * 1000L;

    while (nanosleep(&req, &rem) == -1) {
        if (errno != EINTR) {
            break;
        }
        req = rem;
    }
#endif
}
