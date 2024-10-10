#pragma once

#include "defines.h"

// Copies string and allocated new memory
char* ska_strdup(const char* string);
// Copies string from a void pointer and allocated new memory
char* ska_strdup_from_memory(void* data, usize size);
void ska_strcpy(char* destination, const char* source);
bool ska_strncpy(char* destination, usize sizeInBytes, const char* source, usize maxCount);
void ska_strcat(char* destination, const char* source);
void ska_strncat(char* destination, const char* source, usize sizeInBytes);
// Conversions

const char* ska_bool_to_string(bool value);
char* ska_str_to_lower();
char* ska_str_to_lower_and_underscore_whitespace(char* str);
unsigned char* ska_str_convert_string_to_unsigned_char(const char* value, usize* outSize);

// Will trim text based on a delimiter and return the string before that
char* ska_str_trim(const char* value, char delimiter);
// Will trim text based on a size and return the string as the 'output' param
void ska_str_trim_by_size(const char* value, char* output, usize size);
// Same as 'ska_str_trim' but will replace the 'delimiter' with the 'replacementValue'.
char* ska_str_trim_and_replace(const char* value, char delimiter, const char* replacementValue);
// Removes all instances of the passed in char from the string
void ska_str_remove_char(char* string, char charToRemove);
