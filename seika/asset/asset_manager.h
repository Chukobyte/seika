#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

void se_asset_manager_initialize();
void se_asset_manager_finalize();
// --- Texture --- //
struct SETexture* se_asset_manager_load_texture(const char* fileName, const char* key);
struct SETexture* se_asset_manager_load_texture_ex(const char* fileName, const char* key, const char* wrap_s, const char* wrap_t, bool applyNearestNeighbor);
struct SETexture* se_asset_manager_get_texture(const char* key);
bool se_asset_manager_has_texture(const char* key);
// --- Font --- //
struct SEFont* se_asset_manager_load_font(const char* fileName, const char* key, int size, bool applyNearestNeighbor);
struct SEFont* se_asset_manager_load_font_from_memory(const char* key, void* buffer, size_t bufferSize, int size, bool applyNearestNeighbor);
struct SEFont* se_asset_manager_get_font(const char* key);
bool se_asset_manager_has_font(const char* key);
// -- Audio Source --- //
struct SEAudioSource* se_asset_manager_load_audio_source_wav(const char* fileName, const char* key);
struct SEAudioSource* se_asset_manager_get_audio_source(const char* key);
bool se_asset_manager_has_audio_source(const char* key);

#ifdef __cplusplus
}
#endif
