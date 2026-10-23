#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

void ska_asset_manager_initialize();
void ska_asset_manager_finalize();

#if SKA_RENDERING
struct SkaTexture* ska_asset_manager_load_texture(const char* file_name, const char* key);
struct SkaTexture* ska_asset_manager_load_texture_ex(const char* file_name, const char* key, const char* wrap_s, const char* wrap_t, bool apply_nearest_neighbor);
struct SkaTexture* ska_asset_manager_load_texture_from_memory(const char* key, const void* buffer, usize buffer_size);
struct SkaTexture* ska_asset_manager_get_texture(const char* key);
bool ska_asset_manager_has_texture(const char* key);

struct SkaFont* ska_asset_manager_load_font(const char* file_name, const char* key, int size, bool apply_nearest_neighbor);
struct SkaFont* ska_asset_manager_load_font_from_memory(const char* key, void* buffer, usize buffer_size, int size, bool apply_nearest_neighbor);
struct SkaFont* ska_asset_manager_get_font(const char* key);
bool ska_asset_manager_has_font(const char* key);
#endif // #if SKA_RENDERING

#if SKA_AUDIO
struct SkaAudioSource* ska_asset_manager_load_audio_source_wav(const char* file_name, const char* key);
// Like ska_asset_manager_load_audio_source_wav, but never asserts/aborts: returns the
// already-loaded source if `key` is already registered (instead of asserting it isn't), and
// returns NULL (instead of asserting non-null) if the file can't be read/decoded. For callers
// (e.g. scripting bindings) that can't allow a bad/duplicate path to crash the whole process.
struct SkaAudioSource* ska_asset_manager_try_load_audio_source_wav(const char* file_name, const char* key);
struct SkaAudioSource* ska_asset_manager_get_audio_source(const char* key);
bool ska_asset_manager_has_audio_source(const char* key);
#endif // #if SKA_AUDIO

#ifdef __cplusplus
}
#endif
