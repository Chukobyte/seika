#include "file_system.h"

#ifdef _WIN32
#include <Windows.h>
#include <corecrt.h>
#else
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>

#endif


#include <SDL3/SDL_filesystem.h>

#include "logger.h"
#include "seika/string.h"
#include "seika/memory.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // for fopen
#endif

#define CHAR_ARRAY_MAX_BUFFER_SIZE 256

void ska_fs_get_cwd_array(char* array, usize size) {
    if (getcwd(array, (int) size) != NULL) {
        return;
    }
}

bool ska_fs_chdir(const char* dirPath) {
    char currentWorkingPath[CHAR_ARRAY_MAX_BUFFER_SIZE];
    ska_fs_get_cwd_array(currentWorkingPath, CHAR_ARRAY_MAX_BUFFER_SIZE);
    if (strcmp(currentWorkingPath, dirPath) == 0) {
        ska_logger_warn("Attempting to change to the same directory at path '%s'", currentWorkingPath);
        return false;
    }
    if (chdir(dirPath) == 0) {
        return true;
    }
    ska_logger_error("Failed to change directory to path '%s'", dirPath);
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

usize ska_fs_get_file_size(const char* filePath) {
#ifdef _WIN32
    HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ska_logger_error("Error invalid handle value when getting file size at path '%s'", filePath);
        return 0;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        ska_logger_error("Error getting file size at path '%s'", filePath);
        return 0;
    }

    CloseHandle(hFile);
    return (usize) size.QuadPart;
#else
    struct stat st;
    stat(filePath, &st);
    return (usize) st.st_size;
#endif
}

bool ska_fs_write_to_file(const char* filePath, const char* contents) {
    FILE* fp = fopen(filePath, "w");
    if (fp) {
        fprintf(fp, "%s\n", contents);
        fclose(fp);
        return true;
    }
    return false;
}

char* ska_fs_read_file_contents(const char* filePath, usize* sz) {
    char* buffer = NULL;
    FILE* fp = fopen(filePath, "rb");
    usize readSize = 0;
    if (fp) {
        readSize = ska_fs_get_file_size(filePath);
        // Update buffer
        buffer = (char*)SKA_MEM_ALLOCATE_SIZE(readSize + 1);
        if (buffer != NULL) {
            fread(buffer, 1, readSize, fp);
            buffer[readSize] = '\0';
        }
        fclose(fp);
    }
    if (sz != NULL) {
        *sz = readSize;
    }
    return buffer;
}

char* ska_fs_read_file_contents_without_raw(const char* filePath, usize* sz) {
    char* buffer = NULL;
    FILE* fp = fopen(filePath, "r");
    usize readSize = 0;
    if (fp) {
        readSize = ska_fs_get_file_size(filePath);
        // Update buffer
        buffer = (char*)SKA_MEM_ALLOCATE_SIZE(readSize + 1);
        if (buffer != NULL) {
            usize bytesRead = 0;
            while (bytesRead < readSize) {
                const int maxCount = (int)(readSize - bytesRead + 1);
                if (fgets(buffer + bytesRead, maxCount, fp) == NULL) {
                    break;
                }
                bytesRead += strlen(buffer + bytesRead);
                if (bytesRead > readSize) {
                    // Handle error: file contains more data than expected
                    break;
                }
            }
            buffer[readSize] = '\0';
        }
        fclose(fp);
    }
    if (sz != NULL) {
        *sz = readSize;
    }
    return buffer;
}

bool ska_fs_does_file_exist(const char* filePath) {
    FILE* fp = fopen(filePath, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

bool ska_fs_does_dir_exist(const char* dirPath) {
#if defined(WIN32) || defined(WIN64)
    if (strnlen(dirPath, MAX_PATH + 1) > MAX_PATH) {
        return false;
    }
    DWORD result = GetFileAttributesA(dirPath);
    return (result != INVALID_FILE_ATTRIBUTES && (result & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat stats;
    if (stat(dirPath, &stats) == 0) {
        return S_ISDIR(stats.st_mode);
    }
    return false;
#endif
}

char* ska_fs_get_user_save_path(const char* orgName, const char* applicationName, const char* savePath) {
    char buffer[CHAR_ARRAY_MAX_BUFFER_SIZE];
    char* filePath = SDL_GetPrefPath(orgName, applicationName);
    strcpy(buffer, filePath);
    strcat(buffer, savePath);
    char* fullUserSavePath = ska_strdup(buffer);
    SDL_free(filePath);
    return fullUserSavePath;
}
