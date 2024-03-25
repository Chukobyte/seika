#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "platform.h"
#include "seika/string.h"

#define SKA_ASSERT_FAIL_EXIT_CODE (-1)

void ska_print_assert_err(const char* fmt, ...);

#define SKA_ASSERT(PREDICATE) \
SKA_IF_DEBUG(                                                                         \
if (!(PREDICATE)) {                                                                     \
ska_print_assert_err("Assertion failed in file '%s' on line %u!\n", __FILE__, __LINE__); \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                      \
exit(SKA_ASSERT_FAIL_EXIT_CODE);                                                         \
}                                                                                       \
)

#define SKA_ASSERT_FMT(PREDICATE, FMT, ...)                                            \
SKA_IF_DEBUG(                                                                       \
if (!(PREDICATE)) {                                                                   \
ska_print_assert_err("Assertion failed in file '%s' on line %u!", __FILE__, __LINE__); \
ska_print_assert_err(FMT, ##__VA_ARGS__);                                              \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                    \
exit(SKA_ASSERT_FAIL_EXIT_CODE);                                                       \
}                                                                                     \
)

// Non-fatal asserts
#define SKA_ENSURE(PREDICATE)                                                         \
SKA_IF_DEBUG(                                                                      \
if (!(PREDICATE)) {                                                                  \
static bool hasEnsureTriggered = false;                                              \
if (!hasEnsureTriggered) {                                                           \
ska_print_assert_err("Ensure failed in file '%s' on line %u!\n", __FILE__, __LINE__); \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                   \
hasEnsureTriggered = true;                                                           \
}                                                                                    \
}                                                                                    \
)

#define SKA_ENSURE_FMT(PREDICATE, FMT, ...)                                         \
SKA_IF_DEBUG(                                                                    \
if (!(PREDICATE)) {                                                                \
static bool hasEnsureTriggered = false;                                            \
if (!hasEnsureTriggered) {                                                         \
ska_print_assert_err("Ensure failed in file '%s' on line %u!", __FILE__, __LINE__); \
ska_print_assert_err(FMT, ##__VA_ARGS__);                                           \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                 \
hasEnsureTriggered = true;                                                         \
}                                                                                  \
}                                                                                  \
)

#define SKA_ENSURE_ALWAYS(PREDICATE)                                                  \
SKA_IF_DEBUG(                                                                      \
if (!(PREDICATE)) {                                                                  \
ska_print_assert_err("Ensure failed in file '%s' on line %u!\n", __FILE__, __LINE__); \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                   \
}                                                                                    \
)

#define SKA_ENSURE_ALWAYS_FMT(PREDICATE, FMT, ...)                                  \
SKA_IF_DEBUG(                                                                    \
if (!(PREDICATE)) {                                                                \
ska_print_assert_err("Ensure failed in file '%s' on line %u!", __FILE__, __LINE__); \
ska_print_assert_err(FMT, ##__VA_ARGS__);                                           \
if (se_platform_is_debugger_attached()) { SKA_TRIGGER_BREAKPOINT; }                 \
}                                                                                  \
)

#ifdef __cplusplus
}
#endif
