#include "file_system.h"

#ifdef _WIN32
#include <windows.h>
#include <corecrt.h>
#else
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#endif

#if defined(_WIN32) && !defined(_MSC_VER)
// MinGW/zig (Windows, but not MSVC) still needs _mkdir -- declared in io.h/direct.h just like
// MSVC, not in the POSIX unistd.h/dirent.h the #else branch above pulled in for this compiler.
#include <io.h>
#include <direct.h>
#endif

#include "seika/logger.h"
#include "string.h"
#include "memory.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // for fopen
#endif

#define CHAR_ARRAY_MAX_BUFFER_SIZE 256

void ska_fs_get_cwd_array(char* array, usize size) {
    if (getcwd(array, (int) size) != NULL) {
        return;
    }
}

bool ska_fs_chdir(const char* dir_path) {
    char current_working_path[CHAR_ARRAY_MAX_BUFFER_SIZE];
    ska_fs_get_cwd_array(current_working_path, CHAR_ARRAY_MAX_BUFFER_SIZE);
    if (strcmp(current_working_path, dir_path) == 0) {
        ska_logger_warn("Attempting to change to the same directory at path '%s'", current_working_path);
        return false;
    }
    if (chdir(dir_path) == 0) {
        return true;
    }
    ska_logger_error("Failed to change directory to path '%s'", dir_path);
    return false;
}

char* ska_fs_get_cwd() {
    char cwd[CHAR_ARRAY_MAX_BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return ska_strdup(cwd);
    }
    return NULL;
}

void ska_fs_print_cwd() {
    char cwd[CHAR_ARRAY_MAX_BUFFER_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        ska_logger_info("Current working directory: %s\n", cwd);
    } else {
        ska_logger_error("Not able to get current working directory!");
    }
}

usize ska_fs_get_file_size(const char* file_path) {
#ifdef _WIN32
    HANDLE h_file = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file == INVALID_HANDLE_VALUE) {
        ska_logger_error("Error invalid handle value when getting file size at path '%s'", file_path);
        return 0;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(h_file, &size)) {
        CloseHandle(h_file);
        ska_logger_error("Error getting file size at path '%s'", file_path);
        return 0;
    }

    CloseHandle(h_file);
    return (usize) size.QuadPart;
#else
    struct stat st;
    stat(file_path, &st);
    return (usize) st.st_size;
#endif
}

bool ska_fs_write_to_file(const char* file_path, const char* contents) {
    FILE* fp = fopen(file_path, "w");
    if (fp) {
        fprintf(fp, "%s\n", contents);
        fclose(fp);
        return true;
    }
    return false;
}

bool ska_fs_write_to_file_bytes(const char* file_path, const void* data, usize size) {
    FILE* fp = fopen(file_path, "wb");
    if (!fp) return false;
    const usize written = size > 0 ? fwrite(data, 1, size, fp) : 0;
    fclose(fp);
    return written == size;
}

char* ska_fs_read_file_contents(const char* file_path, usize* sz) {
    char* buffer = NULL;
    FILE* fp = fopen(file_path, "rb");
    usize read_size = 0;
    if (fp) {
        read_size = ska_fs_get_file_size(file_path);
        // Update buffer
        buffer = (char*)SKA_ALLOC_BYTES(read_size + 1);
        if (buffer != NULL) {
            fread(buffer, 1, read_size, fp);
            buffer[read_size] = '\0';
        }
        fclose(fp);
    }
    if (sz != NULL) {
        *sz = read_size;
    }
    return buffer;
}

char* ska_fs_read_file_contents_without_raw(const char* file_path, usize* sz) {
    char* buffer = NULL;
    FILE* fp = fopen(file_path, "r");
    usize read_size = 0;
    if (fp) {
        read_size = ska_fs_get_file_size(file_path);
        // Update buffer
        buffer = (char*)SKA_ALLOC_BYTES(read_size + 1);
        if (buffer != NULL) {
            usize bytes_read = 0;
            while (bytes_read < read_size) {
                const int max_count = (int)(read_size - bytes_read + 1);
                if (fgets(buffer + bytes_read, max_count, fp) == NULL) {
                    break;
                }
                bytes_read += strlen(buffer + bytes_read);
                if (bytes_read > read_size) {
                    // Handle error: file contains more data than expected
                    break;
                }
            }
            buffer[read_size] = '\0';
        }
        fclose(fp);
    }
    if (sz != NULL) {
        *sz = read_size;
    }
    return buffer;
}

bool ska_fs_does_file_exist(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

bool ska_fs_does_dir_exist(const char* dir_path) {
#if defined(WIN32) || defined(WIN64)
    if (strnlen(dir_path, MAX_PATH + 1) > MAX_PATH) {
        return false;
    }
    DWORD result = GetFileAttributesA(dir_path);
    return (result != INVALID_FILE_ATTRIBUTES && (result & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat stats;
    if (stat(dir_path, &stats) == 0) {
        return S_ISDIR(stats.st_mode);
    }
    return false;
#endif
}

bool ska_fs_make_dirs(const char* dir_path) {
    if (!dir_path || !dir_path[0]) return false;
    if (ska_fs_does_dir_exist(dir_path)) return true;

    char buffer[512];
    const usize len = strlen(dir_path);
    if (len >= sizeof(buffer)) return false;
    memcpy(buffer, dir_path, len + 1);

    // Create each parent segment in turn (mkdir -p) -- ignores individual mkdir failures (e.g.
    // "already exists") since the final ska_fs_does_dir_exist check below is authoritative.
    for (usize i = 1; i < len; i++) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            const char saved = buffer[i];
            buffer[i] = '\0';
            if (buffer[0] != '\0' && !ska_fs_does_dir_exist(buffer)) {
#if defined(_WIN32)
                _mkdir(buffer);
#else
                mkdir(buffer, 0755);
#endif
            }
            buffer[i] = saved;
        }
    }
    if (!ska_fs_does_dir_exist(buffer)) {
#if defined(_WIN32)
        _mkdir(buffer);
#else
        mkdir(buffer, 0755);
#endif
    }
    return ska_fs_does_dir_exist(buffer);
}

char* ska_fs_get_user_data_dir(const char* org, const char* app) {
    if (!org || !app) return NULL;
    char base[512];
#if defined(_WIN32)
    const char* appdata = getenv("APPDATA");
    if (!appdata) return NULL;
    snprintf(base, sizeof(base), "%s/%s/%s", appdata, org, app);
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) return NULL;
    snprintf(base, sizeof(base), "%s/Library/Application Support/%s/%s", home, org, app);
#else
    const char* xdg_data_home = getenv("XDG_DATA_HOME");
    if (xdg_data_home && xdg_data_home[0]) {
        snprintf(base, sizeof(base), "%s/%s/%s", xdg_data_home, org, app);
    } else {
        const char* home = getenv("HOME");
        if (!home) return NULL;
        snprintf(base, sizeof(base), "%s/.local/share/%s/%s", home, org, app);
    }
#endif
    if (!ska_fs_make_dirs(base)) return NULL;
    return ska_strdup(base);
}

char** ska_fs_list_dir(const char* dir_path, usize* out_count) {
#ifdef _WIN32
    char pattern[512];
    const usize path_len = strlen(dir_path);
    const bool ends_with_sep = (path_len > 0 && (dir_path[path_len - 1] == '/' || dir_path[path_len - 1] == '\\'));
    if (ends_with_sep) {
        snprintf(pattern, sizeof(pattern), "%s*", dir_path);
    } else {
        snprintf(pattern, sizeof(pattern), "%s\\*", dir_path);
    }

    WIN32_FIND_DATAA ffd;
    HANDLE h_find = FindFirstFileA(pattern, &ffd);
    if (h_find == INVALID_HANDLE_VALUE) { *out_count = 0; return NULL; }

    usize count = 0;
    do {
        if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) count++;
    } while (FindNextFileA(h_find, &ffd));
    FindClose(h_find);

    if (count == 0) { *out_count = 0; return NULL; }

    char** entries = (char**)SKA_ALLOC_BYTES(sizeof(char*) * count);
    h_find = FindFirstFileA(pattern, &ffd);
    usize i = 0;
    do {
        if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0 && i < count) {
            entries[i++] = ska_strdup(ffd.cFileName);
        }
    } while (FindNextFileA(h_find, &ffd));
    FindClose(h_find);

    *out_count = i;
    return entries;
#else
    DIR* dir = opendir(dir_path);
    if (!dir) { *out_count = 0; return NULL; }

    usize count = 0;
    struct dirent* de;
    while ((de = readdir(dir)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) count++;
    }
    rewinddir(dir);

    if (count == 0) { closedir(dir); *out_count = 0; return NULL; }

    char** entries = (char**)SKA_ALLOC_BYTES(sizeof(char*) * count);
    usize i = 0;
    while ((de = readdir(dir)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && i < count) {
            entries[i++] = ska_strdup(de->d_name);
        }
    }
    closedir(dir);
    *out_count = i;
    return entries;
#endif
}

void ska_fs_list_dir_free(char** entries, usize count) {
    if (!entries) return;
    for (usize i = 0; i < count; i++) SKA_FREE(entries[i]);
    SKA_FREE(entries);
}
