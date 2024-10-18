#include "seika/string.h"

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "seika/memory.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // for strcpy and strncat
#endif

char* ska_strdup(const char* string) {
    const usize string_length = strlen(string) + 1;
    char* new_string = (char*)SKA_MEM_ALLOCATE_SIZE(string_length);

    if (new_string == NULL) {
        return NULL;
    }

    return strcpy(new_string, string);
}

char* ska_strdup_from_memory(void* data, usize size) {
    char* newString = (char*)SKA_MEM_ALLOCATE_SIZE(size + 1);
    memcpy(newString, data, size);
    newString[size] = '\0';
    return newString;
}

void ska_strcpy(char* destination, const char* source) {
    strcpy(destination, source);
}

bool ska_strncpy(char* destination, usize sizeInBytes, const char* source, usize maxCount) {
#if defined(WIN32) || defined(WIN64)
    if (strncpy_s(destination, sizeInBytes, source, maxCount) != 0) {
        return false;
    }
#else
    strncpy(destination, source, maxCount);
    if (maxCount > 0) {
        destination[maxCount] = '\0';
    }
#endif
    return true;
}

void ska_strcat(char* destination, const char* source) {
    strcat(destination, source);
}

void ska_strncat(char* destination, const char* source, usize sizeInBytes) {
    strncat(destination, source, sizeInBytes);
}


const char* ska_bool_to_string(bool value) {
    return value == true ? "true" : "false";
}

char* ska_str_to_lower(char* str) {
    for (char* p = str; *p; p++) {
        *p = tolower(*p);
    }
    return str;
}

char* ska_str_to_lower_and_underscore_whitespace(char* str) {
    for (char* p = str; *p; p++) {
        if (*p == ' ') {
            *p = '_';
        } else {
            *p = tolower(*p);
        }
    }
    return str;
}

unsigned char* ska_str_convert_string_to_unsigned_char(const char* value, usize* outSize) {
    const usize stringLength = strlen(value);
    *outSize = stringLength + 1;
    unsigned char* returnValue = (unsigned char*)SKA_MEM_ALLOCATE_SIZE(*outSize);
    for (usize i = 0; i < stringLength; i++) {
        returnValue[i] = (unsigned char)value[i];
    }
    returnValue[stringLength] = '\0';
    return returnValue;
}

char* ska_str_trim(const char* value, char delimiter) {
    char* newStr;
    char* lastExt;
    if (value == NULL) {
        return NULL;
    }
    newStr = (char*)SKA_MEM_ALLOCATE_SIZE(strlen(value) + 1);
    if (newStr == NULL) {
        return NULL;
    }
    strcpy(newStr, value);
    lastExt = strrchr(newStr, delimiter);
    if (lastExt != NULL) {
        *lastExt = '\0';
    }
    return newStr;
}

void ska_str_trim_by_size(const char* value, char* output, usize size) {
    for (usize i = 0; i < size; i++) {
        output[i] = value[i];
    }
    output[size] = '\0';
}

char* ska_str_trim_and_replace(const char* value, char delimiter, const char* replacementValue) {
#define SE_TRIM_PATH_BUFFER 256
    char* newValue = ska_str_trim(value, delimiter);
    char pathBuffer[SE_TRIM_PATH_BUFFER];
    strcpy(pathBuffer, newValue);
    strcat(pathBuffer, replacementValue);
    SKA_MEM_FREE(newValue);
    newValue = ska_strdup(pathBuffer);
    return newValue;
#undef SE_TRIM_PATH_BUFFER
}

void ska_str_remove_char(char* string, char charToRemove) {
    char* dest = string;

    while (*string) {
        if (*string != charToRemove) {
            *dest = *string;
            dest++;
        }
        string++;
    }

    *dest = '\0';
}

char* get_project_archive_name(const char* startingPath) {
    if (startingPath == NULL) {
        return NULL;
    }
    char* validPath = ska_str_trim(startingPath, '.');
    char pathBuffer[256];
    strcpy(pathBuffer, validPath);
    strcat(pathBuffer, ".zip");
    SKA_MEM_FREE(validPath);
    validPath = ska_strdup(pathBuffer);
    return validPath;
}
