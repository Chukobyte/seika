#include "seika/assert.h"

void ska_print_assert_err(const char* fmt, ...) {
#define SKA_PRINT_BUFFER_SIZE 496
    va_list args;
            va_start(args, fmt);
    char str[SKA_PRINT_BUFFER_SIZE];
    memset(str, 0, SKA_PRINT_BUFFER_SIZE);
    ska_strncat(str,  fmt, (sizeof(str) - strlen(str) - 1) );
    ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
    vfprintf(stderr, str, args);
            va_end(args);
#undef SKA_PRINT_BUFFER_SIZE
}
