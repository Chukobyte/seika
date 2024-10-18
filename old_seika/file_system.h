#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

// Change directory
bool ska_fs_chdir(const char* dirPath);
// Get current working directory
char* ska_fs_get_cwd();
// Print current working directory
void ska_fs_print_cwd();

usize ska_fs_get_file_size(const char* filePath);
char* ska_fs_read_file_contents(const char* filePath, usize* sz);
char* ska_fs_read_file_contents_without_raw(const char* filePath, usize* sz);
bool ska_fs_write_to_file(const char* filePath, const char* contents);
bool ska_fs_does_file_exist(const char* filePath);
bool ska_fs_does_dir_exist(const char* dirPath);
char* ska_fs_get_user_save_path(const char* orgName, const char* applicationName, const char* savePath);

#ifdef __cplusplus
}
#endif
