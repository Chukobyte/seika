#include "se_string_util.h"

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "../memory/se_mem.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // for strcpy and strncat
#endif

char* se_strdup(const char* string) {
    const size_t string_length = strlen(string) + 1;
    char* new_string = (char*) SE_MEM_ALLOCATE_SIZE(string_length);

    if (new_string == NULL) {
        return NULL;
    }

    return strcpy(new_string, string);
}

char* se_strdup_from_memory(void* data, size_t size) {
    char* newString = (char*) SE_MEM_ALLOCATE_SIZE(size + 1);
    memcpy(newString, data, size);
    newString[size] = '\0';
    return newString;
}

void se_strcpy(char* destination, const char* source) {
    strcpy(destination, source);
}

bool se_strncpy(char* destination, size_t sizeInBytes, const char* source, size_t maxCount) {
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

void se_strcat(char* destination, const char* source) {
    strcat(destination, source);
}

void se_strncat(char* destination, const char* source, size_t sizeInBytes) {
    strncat(destination, source, sizeInBytes);
}


const char* se_bool_to_string(bool value) {
    return value == true ? "true" : "false";
}

char* se_str_to_lower(char* str) {
    for (char* p = str; *p; p++) {
        *p = tolower(*p);
    }
    return str;
}

char* se_str_to_lower_and_underscore_whitespace(char* str) {
    for (char* p = str; *p; p++) {
        if (*p == ' ') {
            *p = '_';
        } else {
            *p = tolower(*p);
        }
    }
    return str;
}

unsigned char* se_str_convert_string_to_unsigned_char(const char* value, size_t* outSize) {
    const size_t stringLength = strlen(value);
    *outSize = stringLength + 1;
    unsigned char* returnValue = (unsigned char*)SE_MEM_ALLOCATE_SIZE(*outSize);
    for (size_t i = 0; i < stringLength; i++) {
        returnValue[i] = (unsigned char)value[i];
    }
    returnValue[stringLength] = '\0';
    return returnValue;
}

char* se_str_trim(const char* value, char delimiter) {
    char* newStr;
    char* lastExt;
    if (value == NULL) {
        return NULL;
    }
    newStr = SE_MEM_ALLOCATE_SIZE(strlen(value) + 1);
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

void se_str_trim_by_size(const char* value, char* output, size_t size) {
    for (size_t i = 0; i < size; i++) {
        output[i] = value[i];
    }
    output[size] = '\0';
}

char* se_str_trim_and_replace(const char* value, char delimiter, const char* replacementValue) {
#define SE_TRIM_PATH_BUFFER 256
    char* newValue = se_str_trim(value, delimiter);
    char pathBuffer[SE_TRIM_PATH_BUFFER];
    strcpy(pathBuffer, newValue);
    strcat(pathBuffer, replacementValue);
    SE_MEM_FREE(newValue);
    newValue = se_strdup(pathBuffer);
    return newValue;
#undef SE_TRIM_PATH_BUFFER
}

void se_str_remove_char(char* string, char charToRemove) {
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
    char* validPath = se_str_trim(startingPath, '.');
    char pathBuffer[256];
    strcpy(pathBuffer, validPath);
    strcat(pathBuffer, ".zip");
    SE_MEM_FREE(validPath);
    validPath = se_strdup(pathBuffer);
    return validPath;
}
