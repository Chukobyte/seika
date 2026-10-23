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
    char* new_string = (char*)SKA_ALLOC_BYTES(size + 1);
    memcpy(new_string, data, size);
    new_string[size] = '\0';
    return new_string;
}

void ska_strcpy(char* destination, const char* source) {
    strcpy(destination, source);
}

bool ska_strncpy(char* destination, usize size_in_bytes, const char* source, usize max_count) {
#if defined(WIN32) || defined(WIN64)
    if (strncpy_s(destination, size_in_bytes, source, max_count) != 0) {
        return false;
    }
#else
    strncpy(destination, source, max_count);
    if (max_count > 0) {
        destination[max_count] = '\0';
    }
#endif
    return true;
}

void ska_strcat(char* destination, const char* source) {
    strcat(destination, source);
}

void ska_strncat(char* destination, const char* source, usize size_in_bytes) {
    strncat(destination, source, size_in_bytes);
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

unsigned char* ska_str_convert_string_to_unsigned_char(const char* value, usize* out_size) {
    const usize string_length = strlen(value);
    *out_size = string_length + 1;
    unsigned char* return_value = (unsigned char*)SKA_ALLOC_BYTES(*out_size);
    for (usize i = 0; i < string_length; i++) {
        return_value[i] = (unsigned char)value[i];
    }
    return_value[string_length] = '\0';
    return return_value;
}

char* ska_str_trim(const char* value, char delimiter) {
    char* new_str;
    char* last_ext;
    if (value == NULL) {
        return NULL;
    }
    new_str = (char*)SKA_ALLOC_BYTES(strlen(value) + 1);
    if (new_str == NULL) {
        return NULL;
    }
    strcpy(new_str, value);
    last_ext = strrchr(new_str, delimiter);
    if (last_ext != NULL) {
        *last_ext = '\0';
    }
    return new_str;
}

void ska_str_trim_by_size(const char* value, char* output, usize size) {
    for (usize i = 0; i < size; i++) {
        output[i] = value[i];
    }
    output[size] = '\0';
}

char* ska_str_trim_and_replace(const char* value, char delimiter, const char* replacement_value) {
#define SE_TRIM_PATH_BUFFER 256
    char* new_value = ska_str_trim(value, delimiter);
    char path_buffer[SE_TRIM_PATH_BUFFER];
    strcpy(path_buffer, new_value);
    strcat(path_buffer, replacement_value);
    SKA_FREE(new_value);
    new_value = ska_strdup(path_buffer);
    return new_value;
#undef SE_TRIM_PATH_BUFFER
}

void ska_str_remove_char(char* string, char char_to_remove) {
    char* dest = string;

    while (*string) {
        if (*string != char_to_remove) {
            *dest = *string;
            dest++;
        }
        string++;
    }

    *dest = '\0';
}

char* get_project_archive_name(const char* starting_path) {
    if (starting_path == NULL) {
        return NULL;
    }
    char* valid_path = ska_str_trim(starting_path, '.');
    char path_buffer[256];
    strcpy(path_buffer, valid_path);
    strcat(path_buffer, ".zip");
    SKA_FREE(valid_path);
    valid_path = ska_strdup(path_buffer);
    return valid_path;
}

// --- SkaString (Dynamic String with SSO) Implementation ---

SkaStringError ska_string_init(SkaString* str, const char* value, usize len) {
    str->len = 0;
    str->is_heap = false;
    memset(&str->storage, 0, sizeof(str->storage));
    return ska_string_set(str, value, len);
}

void ska_string_init_empty(SkaString* str) {
    str->len = 0;
    str->is_heap = false;
    memset(str->storage.sso, 0, SKA_STRING_SSO_CAPACITY + 1);
}

void ska_string_deinit(SkaString* str) {
  if (str->is_heap && str->storage.heap != NULL) {
    SKA_FREE(str->storage.heap);
    str->storage.heap = NULL;
  }
    str->len = 0;
    str->is_heap = false;
}

const char* ska_string_as_slice(const SkaString* str) {
  return str->is_heap ? str->storage.heap : str->storage.sso;
}

void ska_string_clear(SkaString* str) {
  if (str->is_heap) {
    SKA_FREE(str->storage.heap);
    str->storage.heap = NULL;
    str->is_heap = false;
  }
    memset(str->storage.sso, 0, SKA_STRING_SSO_CAPACITY + 1);
    str->len = 0;
}

SkaStringError ska_string_set(SkaString* str, const char* value, usize len) {
  if (str->is_heap) {
    SKA_FREE(str->storage.heap);
    str->storage.heap = NULL;
    str->is_heap = false;
  }

    str->len = len;

    if (len <= SKA_STRING_SSO_CAPACITY) {
        memcpy(str->storage.sso, value, len);
        memset(str->storage.sso + len, 0, SKA_STRING_SSO_CAPACITY + 1 - len);
    } else {
        char* heap_copy = (char*)SKA_ALLOC_BYTES(len + 1);
        if (!heap_copy) {
            str->len = 0;
            return SkaStringError_OUT_OF_MEMORY;
        }
        memcpy(heap_copy, value, len);
        heap_copy[len] = '\0';
        str->storage.heap = heap_copy;
        str->is_heap = true;
    }

    return SkaStringError_OK;
}

SkaStringError ska_string_append(SkaString* str, const char* value, usize len) {
    const usize new_len = str->len + len;

    if (new_len <= SKA_STRING_SSO_CAPACITY && !str->is_heap) {
      memcpy(str->storage.sso + str->len, value, len);
      str->len = new_len;
      memset(str->storage.sso + new_len, 0, SKA_STRING_SSO_CAPACITY + 1 - new_len);
      return SkaStringError_OK;
    }

    char* buffer = NULL;

    if (str->is_heap) {
      buffer = (char *)ska_mem_global_reallocate(str->storage.heap, new_len + 1);
      if (!buffer) {
        return SkaStringError_OUT_OF_MEMORY;
      }
    } else {
      buffer = (char *)SKA_ALLOC_BYTES(new_len + 1);
      if (!buffer) {
        return SkaStringError_OUT_OF_MEMORY;
      }
      memcpy(buffer, str->storage.sso, str->len);
    }

    memcpy(buffer + str->len, value, len);
    buffer[new_len] = '\0';
    str->storage.heap = buffer;
    str->len = new_len;
    str->is_heap = true;

    return SkaStringError_OK;
}

bool ska_string_eql(const SkaString* a, const SkaString* b) {
    if (a->len != b->len) {
        return false;
    }
    const char* a_data = ska_string_as_slice(a);
    const char* b_data = ska_string_as_slice(b);
    return memcmp(a_data, b_data, a->len) == 0;
}

bool ska_string_eql_cstr(const SkaString* str, const char* cstr, usize len) {
    if (str->len != len) {
        return false;
    }
    const char* str_data = ska_string_as_slice(str);
    return memcmp(str_data, cstr, len) == 0;
}

SkaStringError ska_string_copy(const SkaString* src, SkaString* dest) {
    const char* src_data = ska_string_as_slice(src);
    return ska_string_init(dest, src_data, src->len);
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
      if (str->is_heap) {
        // For heap strings, move in-place
        memmove(str->storage.heap, str->storage.heap + start, new_len);
        str->storage.heap[new_len] = '\0';
      } else {
        // For SSO, move in-place
        memmove(str->storage.sso, str->storage.sso + start, new_len);
        memset(str->storage.sso + new_len, 0,
               SKA_STRING_SSO_CAPACITY + 1 - new_len);
      }
    } else {
        // Only trimming from end: terminate at the new length
        if (str->is_heap) {
          str->storage.heap[new_len] = '\0';
        } else {
          memset(str->storage.sso + new_len, 0,
                 SKA_STRING_SSO_CAPACITY + 1 - new_len);
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
    if (str->is_heap) {
      memmove(str->storage.heap, str->storage.heap + start, new_len);
      str->storage.heap[new_len] = '\0';
    } else {
      memmove(str->storage.sso, str->storage.sso + start, new_len);
      memset(str->storage.sso + new_len, 0, SKA_STRING_SSO_CAPACITY + 1 - new_len);
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

    // Just update length and terminate
    if (str->is_heap) {
      str->storage.heap[end] = '\0';
    } else {
      memset(str->storage.sso + end, 0, SKA_STRING_SSO_CAPACITY + 1 - end);
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
  return str->is_heap ? str->storage.heap : str->storage.sso;
}

bool ska_string_ensure_null_terminated(SkaString* str) {
    // Always true: SSO and heap storage both keep a byte for the terminator
    ska_string_get_writable(str)[str->len] = '\0';
    return true;
}
