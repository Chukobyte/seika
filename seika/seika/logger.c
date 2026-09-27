#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "string.h"
#include "memory.h"

#define SKA_LOGGER_MAX_QUEUED_MESSAGES 16

static SkaLogLevel current_log_level = SkaLogLevel_ERROR;
static char *queued_log_messages[SKA_LOGGER_MAX_QUEUED_MESSAGES];
static usize queued_log_messages_count = 0;
static usize queued_log_messages_index = 0;

void ska_logger_set_level(SkaLogLevel level) { current_log_level = level; }

void ska_logger_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (current_log_level <= SkaLogLevel_ERROR) {
      char str[SKA_LOG_BUFFER_SIZE];
      const char *log_level_text = "[ERROR] ";
      ska_strcpy(str, log_level_text);
      ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1));
      ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1));
      vfprintf(stderr, str, args);
    }
    va_end(args);
}

void ska_logger_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (current_log_level <= SkaLogLevel_WARN) {
      char str[SKA_LOG_BUFFER_SIZE];
      const char *log_level_text = "[WARN] ";
      ska_strcpy(str, log_level_text);
      ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1));
      ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1));
      vprintf(str, args);
    }
    va_end(args);
}

void ska_logger_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (current_log_level <= SkaLogLevel_INFO) {
      char str[SKA_LOG_BUFFER_SIZE];
      const char *log_level_text = "[INFO] ";
      ska_strcpy(str, log_level_text);
      ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1));
      ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1));
      vprintf(str, args);
    }
    va_end(args);
}

void ska_logger_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (current_log_level <= SkaLogLevel_DEBUG) {
      char str[SKA_LOG_BUFFER_SIZE];
      const char *log_level_text = "[DEBUG] ";
      ska_strcpy(str, log_level_text);
      ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1));
      ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1));
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

void ska_logger_message_and_flush(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char str[SKA_LOG_BUFFER_SIZE];
    memset(str, 0, SKA_LOG_BUFFER_SIZE);
    ska_strncat(str,  fmt, (sizeof(str) - strlen(str) - 1) );
    ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1) );
    vfprintf(stdout, str, args);
    fflush(stdout);
    va_end(args);
}

void ska_logger_internal_queue_message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (current_log_level <= SkaLogLevel_WARN) {
      char str[SKA_LOG_BUFFER_SIZE];
      const char *log_level_text = "[WARN] ";
      ska_strcpy(str, log_level_text);
      ska_strncat(str, fmt, (sizeof(str) - strlen(str) - 1));
      ska_strncat(str, "\n", (sizeof(str) - strlen(str) - 1));
      vsnprintf(str, sizeof(str), str, args);

      usize current_index;
      if (queued_log_messages_index + 1 >= SKA_LOGGER_MAX_QUEUED_MESSAGES) {
        queued_log_messages_index = 0;
        current_index = 0;
      } else {
        current_index = queued_log_messages_index++;
        queued_log_messages_count++;
      }
      // Freeing in case we looped around
      SKA_FREE(queued_log_messages[current_index]);
      queued_log_messages[current_index] = ska_strdup(str);
    }
    va_end(args);
}

bool ska_logger_internal_print_queue() {
  if (queued_log_messages_count > 0) {
    for (usize i = 0; i < queued_log_messages_count; i++) {
      ska_logger_message(queued_log_messages[i]);
      SKA_FREE(queued_log_messages[i]);
      queued_log_messages[i] = NULL;
    }
    queued_log_messages_count = 0;
    queued_log_messages_index = 0;
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
