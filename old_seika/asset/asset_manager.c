#include "asset_manager.h"

#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/data_structures/hash_map_string.h"
#include "seika/rendering/texture.h"
#include "seika/rendering/font.h"
#include "seika/audio/audio.h"

static SkaStringHashMap* texturesMap = NULL;
static SkaStringHashMap* fontMap = NULL;
static SkaStringHashMap* audioSourceMap = NULL;

void ska_asset_manager_initialize() {
    texturesMap = ska_string_hash_map_create_default_capacity();
    fontMap = ska_string_hash_map_create_default_capacity();
    audioSourceMap = ska_string_hash_map_create_default_capacity();
}

void ska_asset_manager_finalize() {
    ska_string_hash_map_destroy(texturesMap);
    ska_string_hash_map_destroy(fontMap);
    ska_string_hash_map_destroy(audioSourceMap);
}

// --- Texture --- //
SkaTexture* ska_asset_manager_load_texture(const char* fileName, const char* key) {
    SKA_ASSERT(texturesMap != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(texturesMap, fileName), "Already loaded texture at file path '%'s!  Has key '%s'.", fileName, key);
    SkaTexture* texture = ska_texture_create_texture(fileName);
    ska_string_hash_map_add(texturesMap, key, texture, sizeof(SkaTexture));
    SKA_MEM_FREE(texture);
    texture = (SkaTexture*) ska_string_hash_map_get(texturesMap, key);
    return texture;
}

SkaTexture* ska_asset_manager_load_texture_ex(const char* fileName, const char* key, const char* wrap_s, const char* wrap_t, bool applyNearestNeighbor) {
    SKA_ASSERT(texturesMap != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(texturesMap, fileName), "Already loaded texture at file path '%'s!  Has key '%s'.", fileName, key);
    SkaTexture* texture = ska_texture_create_texture2(
                             fileName,
                             ska_texture_wrap_string_to_int(wrap_s),
                             ska_texture_wrap_string_to_int(wrap_t),
                             applyNearestNeighbor
                         );
    ska_string_hash_map_add(texturesMap, key, texture, sizeof(SkaTexture));
    SKA_MEM_FREE(texture);
    texture = (SkaTexture*) ska_string_hash_map_get(texturesMap, key);
    return texture;
}

SkaTexture* ska_asset_manager_get_texture(const char* key) {
    return (SkaTexture*) ska_string_hash_map_get(texturesMap, key);
}

bool ska_asset_manager_has_texture(const char* key) {
    return ska_string_hash_map_has(texturesMap, key);
}

// --- Font --- //
SkaFont* ska_asset_manager_load_font(const char* fileName, const char* key, int size, bool applyNearestNeighbor) {
    SKA_ASSERT_FMT(!ska_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SkaFont* font = ska_font_create_font(fileName, size, applyNearestNeighbor);
    SKA_ASSERT_FMT(font != NULL, "Failed to load font! file_name: '%s', key: '%s', size: '%d'", fileName, key, size);
    ska_string_hash_map_add(fontMap, key, font, sizeof(SkaFont));
    SKA_MEM_FREE(font);
    font = (SkaFont*) ska_string_hash_map_get(fontMap, key);
    return font;
}

SkaFont* ska_asset_manager_load_font_from_memory(const char* key, void* buffer, usize bufferSize, int size, bool applyNearestNeighbor) {
    SKA_ASSERT_FMT(!ska_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SkaFont* font = ska_font_create_font_from_memory(buffer, bufferSize, size, applyNearestNeighbor);
    SKA_ASSERT_FMT(font != NULL, "Failed to load font! key: '%s', size: '%d'", key, size);
    ska_string_hash_map_add(fontMap, key, font, sizeof(SkaFont));
    SKA_MEM_FREE(font);
    font = (SkaFont*) ska_string_hash_map_get(fontMap, key);
    return  font;
}


SkaFont* ska_asset_manager_get_font(const char* key) {
    return (SkaFont*) ska_string_hash_map_get(fontMap, key);
}

bool ska_asset_manager_has_font(const char* key) {
    return ska_string_hash_map_has(fontMap, key);
}

// --- Audio Source --- //
SkaAudioSource* ska_asset_manager_load_audio_source_wav(const char* fileName, const char* key) {
    SKA_ASSERT(audioSourceMap != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(audioSourceMap, fileName), "Already loaded audio source at file path '%'s!  Has key '%s'.", fileName, key);
    SkaAudioSource* newAudioSource = ska_audio_load_audio_source_wav(fileName);
    SKA_ASSERT_FMT(newAudioSource != NULL, "Audio source is null!  file_name = '%s', key = '%s'", fileName, key);
    ska_string_hash_map_add(audioSourceMap, key, newAudioSource, sizeof(SkaAudioSource));
    SKA_MEM_FREE(newAudioSource);
    return newAudioSource;
}

SkaAudioSource* ska_asset_manager_get_audio_source(const char* key) {
    return (SkaAudioSource*) ska_string_hash_map_get(audioSourceMap, key);
}

bool ska_asset_manager_has_audio_source(const char* key) {
    return ska_string_hash_map_has(audioSourceMap, key);
}
