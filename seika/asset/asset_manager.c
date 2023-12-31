#include "asset_manager.h"

#include "../data_structures/se_hash_map_string.h"
#include "../rendering/texture.h"
#include "../rendering/font.h"
#include "../audio/audio.h"
#include "../memory/se_mem.h"
#include "../utils/se_assert.h"

#define INITIAL_ASSET_HASHMAP_SIZE 8

SEStringHashMap* texturesMap = NULL;
SEStringHashMap* fontMap = NULL;
SEStringHashMap* audioSourceMap = NULL;

void se_asset_manager_initialize() {
    texturesMap = se_string_hash_map_create(INITIAL_ASSET_HASHMAP_SIZE);
    fontMap = se_string_hash_map_create(INITIAL_ASSET_HASHMAP_SIZE);
    audioSourceMap = se_string_hash_map_create(INITIAL_ASSET_HASHMAP_SIZE);
}

void se_asset_manager_finalize() {
    se_string_hash_map_destroy(texturesMap);
    se_string_hash_map_destroy(fontMap);
    se_string_hash_map_destroy(audioSourceMap);
}

// --- Texture --- //
SETexture* se_asset_manager_load_texture(const char* fileName, const char* key) {
    SE_ASSERT(texturesMap != NULL);
    SE_ASSERT_FMT(!se_string_hash_map_has(texturesMap, fileName), "Already loaded texture at file path '%'s!  Has key '%s'.", fileName, key);
    SETexture* texture = se_texture_create_texture(fileName);
    se_string_hash_map_add(texturesMap, key, texture, sizeof(SETexture));
    SE_MEM_FREE(texture);
    texture = (SETexture*) se_string_hash_map_get(texturesMap, key);
    return texture;
}

SETexture* se_asset_manager_load_texture_ex(const char* fileName, const char* key, const char* wrap_s, const char* wrap_t, bool applyNearestNeighbor) {
    SE_ASSERT(texturesMap != NULL);
    SE_ASSERT_FMT(!se_string_hash_map_has(texturesMap, fileName), "Already loaded texture at file path '%'s!  Has key '%s'.", fileName, key);
    SETexture* texture = se_texture_create_texture_ex(
                             fileName,
                             se_texture_wrap_string_to_int(wrap_s),
                             se_texture_wrap_string_to_int(wrap_t),
                             applyNearestNeighbor
                         );
    se_string_hash_map_add(texturesMap, key, texture, sizeof(SETexture));
    SE_MEM_FREE(texture);
    texture = (SETexture*) se_string_hash_map_get(texturesMap, key);
    return texture;
}

SETexture* se_asset_manager_get_texture(const char* key) {
    return (SETexture*) se_string_hash_map_get(texturesMap, key);
}

bool se_asset_manager_has_texture(const char* key) {
    return se_string_hash_map_has(texturesMap, key);
}

// --- Font --- //
SEFont* se_asset_manager_load_font(const char* fileName, const char* key, int size, bool applyNearestNeighbor) {
    SE_ASSERT_FMT(!se_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SEFont* font = ska_font_create_font(fileName, size, applyNearestNeighbor);
    SE_ASSERT_FMT(font != NULL, "Failed to load font! file_name: '%s', key: '%s', size: '%d'", fileName, key, size);
    se_string_hash_map_add(fontMap, key, font, sizeof(SEFont));
    SE_MEM_FREE(font);
    font = (SEFont*) se_string_hash_map_get(fontMap, key);
    return font;
}

SEFont* se_asset_manager_load_font_from_memory(const char* key, void* buffer, size_t bufferSize, int size, bool applyNearestNeighbor) {
    SE_ASSERT_FMT(!se_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SEFont* font = ska_font_create_font_from_memory(buffer, bufferSize, size, applyNearestNeighbor);
    SE_ASSERT_FMT(font != NULL, "Failed to load font! key: '%s', size: '%d'", key, size);
    se_string_hash_map_add(fontMap, key, font, sizeof(SEFont));
    SE_MEM_FREE(font);
    font = (SEFont*) se_string_hash_map_get(fontMap, key);
    return  font;
}


SEFont* se_asset_manager_get_font(const char* key) {
    return (SEFont*) se_string_hash_map_get(fontMap, key);
}

bool se_asset_manager_has_font(const char* key) {
    return se_string_hash_map_has(fontMap, key);
}

// --- Audio Source --- //
SEAudioSource* se_asset_manager_load_audio_source_wav(const char* fileName, const char* key) {
    SE_ASSERT(audioSourceMap != NULL);
    SE_ASSERT_FMT(!se_string_hash_map_has(audioSourceMap, fileName), "Already loaded audio source at file path '%'s!  Has key '%s'.", fileName, key);
    SEAudioSource* newAudioSource = se_audio_load_audio_source_wav(fileName);
    SE_ASSERT_FMT(newAudioSource != NULL, "Audio source is null!  file_name = '%s', key = '%s'", fileName, key);
    se_string_hash_map_add(audioSourceMap, key, newAudioSource, sizeof(SEAudioSource));
    SE_MEM_FREE(newAudioSource);
    return newAudioSource;
}

SEAudioSource* se_asset_manager_get_audio_source(const char* key) {
    return (SEAudioSource*) se_string_hash_map_get(audioSourceMap, key);
}

bool se_asset_manager_has_audio_source(const char* key) {
    return se_string_hash_map_has(audioSourceMap, key);
}
