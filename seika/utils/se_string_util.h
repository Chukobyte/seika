#pragma once

#include <stddef.h>
#include <stdbool.h>

// Copies string and allocated new memory
char* se_strdup(const char* string);
// Copies string from a void pointer and allocated new memory
char* se_strdup_from_memory(void* data, size_t size);
void se_strcpy(char* destination, const char* source);
bool se_strncpy(char* destination, size_t sizeInBytes, const char* source, size_t maxCount);
void se_strcat(char* destination, const char* source);
void se_strncat(char* destination, const char* source, size_t sizeInBytes);
// Conversions

const char* se_bool_to_string(bool value);
char* se_str_to_lower();
char* se_str_to_lower_and_underscore_whitespace(char* str);
unsigned char* se_str_convert_string_to_unsigned_char(const char* value, size_t* outSize);

// Will trim text based on a delimiter and return the string before that
char* se_str_trim(const char* value, char delimiter);
// Will trim text based on a size and return the string as the 'output' param
void se_str_trim_by_size(const char* value, char* output, size_t size);
// Same as 'se_str_trim' but will replace the 'delimiter' with the 'replacementValue'.
char* se_str_trim_and_replace(const char* value, char delimiter, const char* replacementValue);
// Removes all instances of the passed in char from the string
void se_str_remove_char(char* string, char charToRemove);
