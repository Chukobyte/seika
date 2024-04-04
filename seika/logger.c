#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "seika/string.h"
#include "memory.h"

#define SKA_LOGGER_MAX_QUEUED_MESSAGES 16

static SkaLogLevel currentLogLevel = SkaLogLevel_ERROR;
static char* queuedLogMessages[SKA_LOGGER_MAX_QUEUED_MESSAGES];
static usize queuedLogMessagesCount = 0;
static usize queuedLogMessagesIndex = 0;

void ska_logger_set_level(SkaLogLevel level) {
    currentLogLevel = level;
}

void ska_logger_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (currentLogLevel <= SkaLogLevel_ERROR) {
        char str[SKA_LOG_BUFFER_SIZE];
        const char* logLevelText = "[ERROR] ";
        ska_strcpy(str, logLevelText);
        ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1) );
        ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
        vfprintf(stderr, str, args);
    }
    va_end(args);
}

void ska_logger_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (currentLogLevel <= SkaLogLevel_WARN) {
        char str[SKA_LOG_BUFFER_SIZE];
        const char* logLevelText = "[WARN] ";
        ska_strcpy(str, logLevelText);
        ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1) );
        ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
        vprintf(str, args);
    }
    va_end(args);
}

void ska_logger_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (currentLogLevel <= SkaLogLevel_INFO) {
        char str[SKA_LOG_BUFFER_SIZE];
        const char* logLevelText = "[INFO] ";
        ska_strcpy(str, logLevelText);
        ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1) );
        ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
        vprintf(str, args);
    }
    va_end(args);
}

void ska_logger_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (currentLogLevel <= SkaLogLevel_DEBUG) {
        char str[SKA_LOG_BUFFER_SIZE];
        const char* logLevelText = "[DEBUG] ";
        ska_strcpy(str, logLevelText);
        ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1) );
        ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
        vprintf(str, args);
    }
    va_end(args);
}

void ska_logger_message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char str[SKA_LOG_BUFFER_SIZE];
    memset(str, 0, SKA_LOG_BUFFER_SIZE);
    ska_strncat(str,  fmt, (sizeof(str) - strlen(str) - 1) );
    ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
    vfprintf(stdout, str, args);
    va_end(args);
}

void ska_logger_internal_queue_message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (currentLogLevel <= SkaLogLevel_WARN) {
        char str[SKA_LOG_BUFFER_SIZE];
        const char* logLevelText = "[WARN] ";
        ska_strcpy(str, logLevelText);
        ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1) );
        ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
        vsnprintf(str, sizeof(str), str, args);

        usize currentIndex;
        if (queuedLogMessagesIndex + 1 >= SKA_LOGGER_MAX_QUEUED_MESSAGES) {
            queuedLogMessagesIndex = 0;
            currentIndex = 0;
        } else {
            currentIndex = queuedLogMessagesIndex++;
            queuedLogMessagesCount++;
        }
        // Freeing in case we looped around
        SKA_MEM_FREE(queuedLogMessages[currentIndex]);
        queuedLogMessages[currentIndex] = ska_strdup(str);
    }
    va_end(args);
}

bool ska_logger_internal_print_queue() {
    if (queuedLogMessagesCount > 0) {
        for (usize i = 0; i < queuedLogMessagesCount; i++) {
            ska_logger_message(queuedLogMessages[i]);
            SKA_MEM_FREE(queuedLogMessages[i]);
            queuedLogMessages[i] = NULL;
        }
        queuedLogMessagesCount = 0;
        queuedLogMessagesIndex = 0;
        return true;
    }
    return false;
}

const char* ska_logger_get_log_level_string(SkaLogLevel level) {
    switch (level) {
    case SkaLogLevel_DEBUG:
        return "debug";
    case SkaLogLevel_INFO:
        return "info";
    case SkaLogLevel_WARN:
        return "warn";
    case SkaLogLevel_ERROR:
        return "error";
    }
    return NULL;
}

SkaLogLevel ska_logger_get_log_level_enum(const char* level) {
    if (strcmp(level, "debug") == 0) {
        return SkaLogLevel_DEBUG;
    } else if (strcmp(level, "info") == 0) {
        return SkaLogLevel_INFO;
    } else if (strcmp(level, "warn") == 0) {
        return SkaLogLevel_WARN;
    } else if (strcmp(level, "error") == 0) {
        return SkaLogLevel_ERROR;
    }
    return SkaLogLevel_ERROR;
}
