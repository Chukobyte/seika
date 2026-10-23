#include "platform.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <sys/ptrace.h>
#endif

bool se_platform_is_debugger_attached() {
#if defined(_WIN32) || defined(_WIN64)
    return IsDebuggerPresent();
#elif defined(__linux__)
    return (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1);
#endif
    return false;
}
