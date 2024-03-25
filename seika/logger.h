#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#ifndef SKA_LOG_BUFFER_SIZE
#define SKA_LOG_BUFFER_SIZE 512
#endif

typedef enum SkaLogLevel {
    SkaLogLevel_DEBUG = 0,
    SkaLogLevel_INFO = 1,
    SkaLogLevel_WARN = 2,
    SkaLogLevel_ERROR = 3
} SkaLogLevel;

void ska_logger_set_level(SkaLogLevel level);
void ska_logger_error(const char* fmt, ...);
void ska_logger_warn(const char* fmt, ...);
void ska_logger_info(const char* fmt, ...);
void ska_logger_debug(const char* fmt, ...);
void ska_logger_message(const char* fmt, ...);

void ska_logger_internal_queue_message(const char* fmt, ...);
bool ska_logger_internal_print_queue();

const char* ska_logger_get_log_level_string(SkaLogLevel level);
SkaLogLevel ska_logger_get_log_level_enum(const char* level);

#ifdef __cplusplus
}
#endif
