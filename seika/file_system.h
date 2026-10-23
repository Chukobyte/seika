#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

// Change directory
bool ska_fs_chdir(const char *dir_path);
// Get current working directory
char* ska_fs_get_cwd();
// Print current working directory
void ska_fs_print_cwd();

usize ska_fs_get_file_size(const char *file_path);
char *ska_fs_read_file_contents(const char *file_path, usize *sz);
char *ska_fs_read_file_contents_without_raw(const char *file_path, usize *sz);
bool ska_fs_write_to_file(const char *file_path, const char *contents);
// Byte-exact write (opens in binary mode, no fprintf/NUL-terminator assumptions, no trailing
// newline) -- use this instead of ska_fs_write_to_file for content that isn't a NUL-terminated
// text string (may itself contain embedded NUL bytes).
bool ska_fs_write_to_file_bytes(const char *file_path, const void *data, usize size);
bool ska_fs_does_file_exist(const char *file_path);
bool ska_fs_does_dir_exist(const char *dir_path);
// Creates `dir_path` and any missing parent directories (like `mkdir -p`). Returns true if the
// directory exists on return, whether it was just created or already there.
bool ska_fs_make_dirs(const char *dir_path);

// Resolves (and creates, if missing) a platform-correct, per-user, per-app writable directory
// for save data / settings: Windows `%APPDATA%/org/app`, macOS `~/Library/Application
// Support/org/app`, Linux `$XDG_DATA_HOME/org/app` or `~/.local/share/org/app`. Returns a
// heap-allocated path (free with SKA_FREE) with no trailing separator, or NULL if the platform's
// base directory env var isn't set or the directory couldn't be created.
char *ska_fs_get_user_data_dir(const char *org, const char *app);

// Same as ska_fs_get_user_data_dir, but for re-creatable cached data (downloads, build artifacts):
// Windows `%LOCALAPPDATA%/org/app`, macOS `~/Library/Caches/org/app`, Linux
// `$XDG_CACHE_HOME/org/app` or `~/.cache/org/app`. Forward-slash separated.
char *ska_fs_get_user_cache_dir(const char *org, const char *app);

// Returns a heap-allocated array of heap-allocated filenames (not full paths) in dirPath.
// *out_count is set to the number of entries. Returns NULL on error or empty dir.
// Free the result with ska_fs_list_dir_free().
char **ska_fs_list_dir(const char *dir_path, usize *out_count);
void ska_fs_list_dir_free(char** entries, usize count);

#ifdef __cplusplus
}
#endif
