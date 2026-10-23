#pragma once

#include "memory/memory.h"

// Define the SSO capacity (Small String Optimization)
#define SKA_STRING_SSO_CAPACITY 22

// Copies string and allocated new memory
char* ska_strdup(const char* string);
char* ska_strdup_a(SkaAllocator allocator, const char* string);
// Copies string from a void pointer and allocated new memory
char* ska_strdup_from_memory(void* data, usize size);
void ska_strcpy(char* destination, const char* source);
bool ska_strncpy(char *destination, usize size_in_bytes, const char *source,
                 usize max_count);
void ska_strcat(char* destination, const char* source);
void ska_strncat(char *destination, const char *source, usize size_in_bytes);
// Searches for `\0` and returns the length up to that point
usize ska_strlen(const char* str);

// Conversions

const char* ska_bool_to_string(bool value);
char* ska_str_to_lower(char* str);
char* ska_str_to_lower_and_underscore_whitespace(char* str);
unsigned char *ska_str_convert_string_to_unsigned_char(const char *value,
                                                       usize *out_size);

// Will trim text based on a delimiter and return the string before that
char* ska_str_trim(const char* value, char delimiter);
// Will trim text based on a size and return the string as the 'output' param
void ska_str_trim_by_size(const char* value, char* output, usize size);
// Same as 'ska_str_trim' but will replace the 'delimiter' with the 'replacementValue'.
char *ska_str_trim_and_replace(const char *value, char delimiter,
                               const char *replacement_value);
// Removes all instances of the passed in char from the string
void ska_str_remove_char(char *string, char char_to_remove);

// Dynamic String

// Error codes for string operations
typedef enum SkaStringError {
    SkaStringError_OK = 0,
    SkaStringError_OUT_OF_MEMORY,
    SkaStringError_INVALID_OPERATION,
} SkaStringError;

typedef struct SkaString {
    usize len;
    bool is_heap;
    union {
        char sso[SKA_STRING_SSO_CAPACITY + 1]; // +1: contents are always null-terminated
        char* heap;
    } storage;
} SkaString;

// Initialize a new string with the given value
SkaStringError ska_string_init(SkaString* str, const char* value, usize len);
// Initialize an empty string
void ska_string_init_empty(SkaString* str);
// Cleanup/free the string
void ska_string_deinit(SkaString* str);
// Get a read-only slice of the string
const char* ska_string_as_slice(const SkaString* str);
// Clear the string (free heap memory if allocated, reset to empty)
void ska_string_clear(SkaString* str);
// Set the string to a new value (replaces current content)
SkaStringError ska_string_set(SkaString* str, const char* value, usize len);
// Append to the string
SkaStringError ska_string_append(SkaString* str, const char* value, usize len);
// Compare two strings for equality
bool ska_string_eql(const SkaString* a, const SkaString* b);
// Compare string with C string
bool ska_string_eql_cstr(const SkaString* str, const char* cstr, usize len);
// Create a duplicate of the string
SkaStringError ska_string_copy(const SkaString* src, SkaString* dest);

// --- New utility functions for parsing ---

// Initialize from a C string (null-terminated)
SkaStringError ska_string_init_cstr(SkaString* str, const char* cstr);
// Initialize from a substring (no null terminator required)
SkaStringError ska_string_init_substring(SkaString* str, const char* start, usize len);

// Trim whitespace from both ends (returns new length)
usize ska_string_trim_whitespace(SkaString* str);
// Trim whitespace from start only (returns new length)
usize ska_string_trim_start(SkaString* str);
// Trim whitespace from end only (returns new length)
usize ska_string_trim_end(SkaString* str);

// Get a substring view (does not allocate, returns slice into original)
// Note: The returned pointer is only valid as long as the source string is valid
const char* ska_string_substring_view(const SkaString* str, usize start, usize len, usize* out_len);

// Copy a substring into a new SkaString
SkaStringError ska_string_substring_copy(const SkaString* src, usize start, usize len, SkaString* dest);

// Find first occurrence of character (returns index, or -1 if not found)
i64 ska_string_find_char(const SkaString* str, char ch);
// Find last occurrence of character
i64 ska_string_find_char_last(const SkaString* str, char ch);

// Check if string starts with prefix
bool ska_string_starts_with(const SkaString* str, const char* prefix, usize prefix_len);
// Check if string ends with suffix
bool ska_string_ends_with(const SkaString* str, const char* suffix, usize suffix_len);

// Check if character is whitespace
static inline bool ska_char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Get writable pointer (for in-place modifications)
// WARNING: Only use this if you know what you're doing - doesn't null-terminate!
char* ska_string_get_writable(SkaString* str);

// Re-terminates after writes through ska_string_get_writable; always returns true
// (every SkaString keeps room for the terminator, so ska_string_as_slice is a valid C string)
bool ska_string_ensure_null_terminated(SkaString* str);
