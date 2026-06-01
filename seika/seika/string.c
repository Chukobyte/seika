#include "string.h"

#include <string.h>
#include <ctype.h>

#include "seika/memory.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // for strcpy and strncat
#endif

char* ska_strdup(const char* string) {
    const usize string_length = strlen(string) + 1;
    char* new_string = (char*)SKA_ALLOC_BYTES(string_length);

    if (new_string == NULL) {
        return NULL;
    }

    return strcpy(new_string, string);
}

char* ska_strdup_a(SkaAllocator allocator, const char* string) {
    const usize string_length = strlen(string) + 1;
    char* new_string = (char*)SKA_ALLOC_BYTES_A(allocator, string_length);

    if (new_string == NULL) {
        return NULL;
    }

    return strcpy(new_string, string);
}

char* ska_strdup_from_memory(void* data, usize size) {
    char* newString = (char*)SKA_ALLOC_BYTES(size + 1);
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

usize ska_strlen(const char* str) {
    return strlen(str);
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
    unsigned char* returnValue = (unsigned char*)SKA_ALLOC_BYTES(*outSize);
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
    newStr = (char*)SKA_ALLOC_BYTES(strlen(value) + 1);
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
    SKA_FREE(newValue);
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
    SKA_FREE(validPath);
    validPath = ska_strdup(pathBuffer);
    return validPath;
}

// --- SkaString (Dynamic String with SSO) Implementation ---

SkaStringError ska_string_init(SkaString* str, const char* value, usize len) {
    str->len = 0;
    str->isHeap = false;
    memset(&str->storage, 0, sizeof(str->storage));
    return ska_string_set(str, value, len);
}

void ska_string_init_empty(SkaString* str) {
    str->len = 0;
    str->isHeap = false;
    memset(str->storage.sso, 0, SKA_STRING_SSO_CAPACITY);
}

void ska_string_deinit(SkaString* str) {
    if (str->isHeap && str->storage.heap != NULL) {
        SKA_FREE(str->storage.heap);
        str->storage.heap = NULL;
    }
    str->len = 0;
    str->isHeap = false;
}

const char* ska_string_as_slice(const SkaString* str) {
    return str->isHeap ? str->storage.heap : str->storage.sso;
}

void ska_string_clear(SkaString* str) {
    if (str->isHeap) {
        SKA_FREE(str->storage.heap);
        str->storage.heap = NULL;
        str->isHeap = false;
        memset(str->storage.sso, 0, SKA_STRING_SSO_CAPACITY);
    }
    str->len = 0;
}

SkaStringError ska_string_set(SkaString* str, const char* value, usize len) {
    if (str->isHeap) {
        SKA_FREE(str->storage.heap);
        str->storage.heap = NULL;
        str->isHeap = false;
    }

    str->len = len;

    if (len <= SKA_STRING_SSO_CAPACITY) {
        memcpy(str->storage.sso, value, len);
        if (len < SKA_STRING_SSO_CAPACITY) {
            memset(str->storage.sso + len, 0, SKA_STRING_SSO_CAPACITY - len);
        }
    } else {
        char* heapCopy = (char*)SKA_ALLOC_BYTES(len);
        if (!heapCopy) {
            str->len = 0;
            return SkaStringError_OUT_OF_MEMORY;
        }
        memcpy(heapCopy, value, len);
        str->storage.heap = heapCopy;
        str->isHeap = true;
    }

    return SkaStringError_OK;
}

SkaStringError ska_string_append(SkaString* str, const char* value, usize len) {
    const usize newLen = str->len + len;

    if (newLen <= SKA_STRING_SSO_CAPACITY && !str->isHeap) {
        memcpy(str->storage.sso + str->len, value, len);
        str->len = newLen;
        if (newLen < SKA_STRING_SSO_CAPACITY) {
            memset(str->storage.sso + newLen, 0, SKA_STRING_SSO_CAPACITY - newLen);
        }
        return SkaStringError_OK;
    }

    char* buffer = NULL;

    if (str->isHeap) {
        buffer = (char*)ska_mem_global_reallocate(str->storage.heap, newLen);
        if (!buffer) {
            return SkaStringError_OUT_OF_MEMORY;
        }
    } else {
        buffer = (char*)SKA_ALLOC_BYTES(newLen);
        if (!buffer) {
            return SkaStringError_OUT_OF_MEMORY;
        }
        memcpy(buffer, str->storage.sso, str->len);
    }

    memcpy(buffer + str->len, value, len);
    str->storage.heap = buffer;
    str->len = newLen;
    str->isHeap = true;

    return SkaStringError_OK;
}

bool ska_string_eql(const SkaString* a, const SkaString* b) {
    if (a->len != b->len) {
        return false;
    }
    const char* aData = ska_string_as_slice(a);
    const char* bData = ska_string_as_slice(b);
    return memcmp(aData, bData, a->len) == 0;
}

bool ska_string_eql_cstr(const SkaString* str, const char* cstr, usize len) {
    if (str->len != len) {
        return false;
    }
    const char* strData = ska_string_as_slice(str);
    return memcmp(strData, cstr, len) == 0;
}

SkaStringError ska_string_copy(const SkaString* src, SkaString* dest) {
    const char* srcData = ska_string_as_slice(src);
    return ska_string_init(dest, srcData, src->len);
}

// --- New utility function implementations ---

SkaStringError ska_string_init_cstr(SkaString* str, const char* cstr) {
    return ska_string_init(str, cstr, strlen(cstr));
}

SkaStringError ska_string_init_substring(SkaString* str, const char* start, usize len) {
    return ska_string_init(str, start, len);
}

usize ska_string_trim_whitespace(SkaString* str) {
    if (str->len == 0) return 0;

    const char* data = ska_string_as_slice(str);
    usize start = 0;
    usize end = str->len;

    // Find first non-whitespace
    while (start < str->len && ska_char_is_whitespace(data[start])) {
        start++;
    }

    // Find last non-whitespace
    while (end > start && ska_char_is_whitespace(data[end - 1])) {
        end--;
    }

    usize new_len = end - start;

    if (start == 0 && new_len == str->len) {
        // Already trimmed
        return str->len;
    }

    if (new_len == 0) {
        ska_string_clear(str);
        return 0;
    }

    // If we're trimming from start, need to move data
    if (start > 0) {
        if (str->isHeap) {
            // For heap strings, move in-place
            memmove(str->storage.heap, str->storage.heap + start, new_len);
        } else {
            // For SSO, move in-place
            memmove(str->storage.sso, str->storage.sso + start, new_len);
            memset(str->storage.sso + new_len, 0, SKA_STRING_SSO_CAPACITY - new_len);
        }
    } else {
        // Only trimming from end, just update length for heap
        // (SSO doesn't need zeroing if we're only shortening)
        if (!str->isHeap && new_len < SKA_STRING_SSO_CAPACITY) {
            memset(str->storage.sso + new_len, 0, SKA_STRING_SSO_CAPACITY - new_len);
        }
    }

    str->len = new_len;
    return new_len;
}

usize ska_string_trim_start(SkaString* str) {
    if (str->len == 0) return 0;

    const char* data = ska_string_as_slice(str);
    usize start = 0;

    // Find first non-whitespace
    while (start < str->len && ska_char_is_whitespace(data[start])) {
        start++;
    }

    if (start == 0) {
        // Already trimmed
        return str->len;
    }

    usize new_len = str->len - start;

    if (new_len == 0) {
        ska_string_clear(str);
        return 0;
    }

    // Move data to start
    if (str->isHeap) {
        memmove(str->storage.heap, str->storage.heap + start, new_len);
    } else {
        memmove(str->storage.sso, str->storage.sso + start, new_len);
        memset(str->storage.sso + new_len, 0, SKA_STRING_SSO_CAPACITY - new_len);
    }

    str->len = new_len;
    return new_len;
}

usize ska_string_trim_end(SkaString* str) {
    if (str->len == 0) return 0;

    const char* data = ska_string_as_slice(str);
    usize end = str->len;

    // Find last non-whitespace
    while (end > 0 && ska_char_is_whitespace(data[end - 1])) {
        end--;
    }

    if (end == str->len) {
        // Already trimmed
        return str->len;
    }

    if (end == 0) {
        ska_string_clear(str);
        return 0;
    }

    // Just update length
    if (!str->isHeap && end < SKA_STRING_SSO_CAPACITY) {
        memset(str->storage.sso + end, 0, SKA_STRING_SSO_CAPACITY - end);
    }

    str->len = end;
    return end;
}

const char* ska_string_substring_view(const SkaString* str, usize start, usize len, usize* out_len) {
    if (start >= str->len) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    usize actual_len = len;
    if (start + len > str->len) {
        actual_len = str->len - start;
    }

    if (out_len) {
        *out_len = actual_len;
    }

    const char* data = ska_string_as_slice(str);
    return data + start;
}

SkaStringError ska_string_substring_copy(const SkaString* src, usize start, usize len, SkaString* dest) {
    if (start >= src->len) {
        ska_string_init_empty(dest);
        return SkaStringError_OK;
    }

    usize actual_len = len;
    if (start + len > src->len) {
        actual_len = src->len - start;
    }

    const char* data = ska_string_as_slice(src);
    return ska_string_init(dest, data + start, actual_len);
}

i64 ska_string_find_char(const SkaString* str, char ch) {
    const char* data = ska_string_as_slice(str);
    for (usize i = 0; i < str->len; i++) {
        if (data[i] == ch) {
            return (i64)i;
        }
    }
    return -1;
}

i64 ska_string_find_char_last(const SkaString* str, char ch) {
    const char* data = ska_string_as_slice(str);
    for (i64 i = (i64)str->len - 1; i >= 0; i--) {
        if (data[i] == ch) {
            return i;
        }
    }
    return -1;
}

bool ska_string_starts_with(const SkaString* str, const char* prefix, usize prefix_len) {
    if (prefix_len > str->len) {
        return false;
    }
    const char* data = ska_string_as_slice(str);
    return memcmp(data, prefix, prefix_len) == 0;
}

bool ska_string_ends_with(const SkaString* str, const char* suffix, usize suffix_len) {
    if (suffix_len > str->len) {
        return false;
    }
    const char* data = ska_string_as_slice(str);
    return memcmp(data + str->len - suffix_len, suffix, suffix_len) == 0;
}

char* ska_string_get_writable(SkaString* str) {
    return str->isHeap ? str->storage.heap : str->storage.sso;
}

bool ska_string_ensure_null_terminated(SkaString* str) {
    if (str->isHeap) {
        // Heap strings don't have automatic null termination in current implementation
        // Would need to reallocate with len + 1
        return false;
    }

    if (str->len < SKA_STRING_SSO_CAPACITY) {
        str->storage.sso[str->len] = '\0';
        return true;
    }

    return false;
}
