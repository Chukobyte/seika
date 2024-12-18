#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

#if defined(_WIN32) || defined(_WIN64) // Windows
#define SKA_TRIGGER_BREAKPOINT __debugbreak()
#elif defined(__APPLE__) // macOS
#include <signal.h>
#define SKA_TRIGGER_BREAKPOINT raise(SIGTRAP)
#elif defined(__linux__) // Linux
#include <signal.h>
#define SKA_TRIGGER_BREAKPOINT raise(SIGTRAP)
#else
#define SKA_TRIGGER_BREAKPOINT
#endif

bool se_platform_is_debugger_attached();

#ifdef __cplusplus
}
#endif
