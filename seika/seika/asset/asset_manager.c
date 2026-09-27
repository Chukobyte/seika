#include "asset_manager.h"

#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/data_structures/hash_map_string.h"
#include "seika/rendering/texture.h"
#include "seika/rendering/font.h"
#include "seika/audio/audio.h"

static SkaStringHashMap* textures_map = NULL;
static SkaStringHashMap* font_map = NULL;
static SkaStringHashMap* audio_source_map = NULL;

// Idempotent: std.audio and std.renderer (koa/koa/std/audio.c, koa/koa/std/renderer.c) both own
// this shared manager's lifecycle and call initialize/finalize independently, since either can be
// registered without the other. Without the guard, registering both would leak the first pair of
// maps on the second initialize(), and unregistering both would double-free on the second
// finalize().
void ska_asset_manager_initialize() {
    if (textures_map) {
        return;
    }
    textures_map = ska_string_hash_map_create_default_capacity();
    font_map = ska_string_hash_map_create_default_capacity();
    audio_source_map = ska_string_hash_map_create_default_capacity();
}

void ska_asset_manager_finalize() {
    if (!textures_map) {
        return;
    }
    ska_string_hash_map_deinit(textures_map);
    ska_string_hash_map_deinit(font_map);
    ska_string_hash_map_deinit(audio_source_map);
    textures_map = NULL;
    font_map = NULL;
    audio_source_map = NULL;
}

#if SKA_RENDERING
// --- Texture --- //
SkaTexture* ska_asset_manager_load_texture(const char* file_name, const char* key) {
    SKA_ASSERT(textures_map != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(textures_map, file_name), "Already loaded texture at file path '%'s!  Has key '%s'.", file_name, key);
    SkaTexture* texture = ska_texture_create(file_name);
    ska_string_hash_map_add(textures_map, key, texture, sizeof(SkaTexture));
    SKA_FREE(texture);
    texture = (SkaTexture*) ska_string_hash_map_get(textures_map, key);
    return texture;
}

SkaTexture* ska_asset_manager_load_texture_ex(const char* file_name, const char* key, const char* wrap_s, const char* wrap_t, bool apply_nearest_neighbor) {
    SKA_ASSERT(textures_map != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(textures_map, file_name), "Already loaded texture at file path '%'s!  Has key '%s'.", file_name, key);
    SkaTexture* texture = ska_texture_create2(
                             file_name,
                             ska_texture_wrap_string_to_int(wrap_s),
                             ska_texture_wrap_string_to_int(wrap_t),
                             apply_nearest_neighbor
                         );
    ska_string_hash_map_add(textures_map, key, texture, sizeof(SkaTexture));
    SKA_FREE(texture);
    texture = (SkaTexture*) ska_string_hash_map_get(textures_map, key);
    return texture;
}

SkaTexture* ska_asset_manager_load_texture_from_memory(const char* key, const void* buffer, usize buffer_size) {
    SKA_ASSERT(textures_map != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(textures_map, key), "Already loaded texture with key '%s'.", key);
    SkaTexture* texture = ska_texture_create_from_memory(buffer, buffer_size);
    ska_string_hash_map_add(textures_map, key, texture, sizeof(SkaTexture));
    SKA_FREE(texture);
    texture = (SkaTexture*) ska_string_hash_map_get(textures_map, key);
    return texture;
}

SkaTexture* ska_asset_manager_get_texture(const char* key) {
    return (SkaTexture*) ska_string_hash_map_get(textures_map, key);
}

bool ska_asset_manager_has_texture(const char* key) {
    return ska_string_hash_map_has(textures_map, key);
}

// --- Font --- //
SkaFont* ska_asset_manager_load_font(const char* file_name, const char* key, int size, bool apply_nearest_neighbor) {
    SKA_ASSERT_FMT(!ska_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SkaFont* font = ska_font_create_font(file_name, size, apply_nearest_neighbor);
    SKA_ASSERT_FMT(font != NULL, "Failed to load font! file_name: '%s', key: '%s', size: '%d'", file_name, key, size);
    ska_string_hash_map_add(font_map, key, font, sizeof(SkaFont));
    SKA_FREE(font);
    font = (SkaFont*) ska_string_hash_map_get(font_map, key);
    return font;
}

SkaFont* ska_asset_manager_load_font_from_memory(const char* key, void* buffer, usize buffer_size, int size, bool apply_nearest_neighbor) {
    SKA_ASSERT_FMT(!ska_asset_manager_has_font(key), "Font key '%s' already exists!", key);
    SkaFont* font = ska_font_create_font_from_memory(buffer, buffer_size, size, apply_nearest_neighbor);
    SKA_ASSERT_FMT(font != NULL, "Failed to load font! key: '%s', size: '%d'", key, size);
    ska_string_hash_map_add(font_map, key, font, sizeof(SkaFont));
    SKA_FREE(font);
    font = (SkaFont*) ska_string_hash_map_get(font_map, key);
    return  font;
}


SkaFont* ska_asset_manager_get_font(const char* key) {
    return (SkaFont*) ska_string_hash_map_get(font_map, key);
}

bool ska_asset_manager_has_font(const char* key) {
    return ska_string_hash_map_has(font_map, key);
}
#endif // #if SKA_RENDERING

#if SKA_AUDIO
// --- Audio Source --- //
SkaAudioSource* ska_asset_manager_load_audio_source_wav(const char* file_name, const char* key) {
    SKA_ASSERT(audio_source_map != NULL);
    SKA_ASSERT_FMT(!ska_string_hash_map_has(audio_source_map, file_name), "Already loaded audio source at file path '%'s!  Has key '%s'.", file_name, key);
    SkaAudioSource* new_audio_source = ska_audio_load_audio_source_wav(file_name);
    SKA_ASSERT_FMT(new_audio_source != NULL, "Audio source is null!  file_name = '%s', key = '%s'", file_name, key);
    ska_string_hash_map_add(audio_source_map, key, new_audio_source, sizeof(SkaAudioSource));
    // SKA_FREE(newAudioSource);
    return new_audio_source;
}

SkaAudioSource* ska_asset_manager_try_load_audio_source_wav(const char* file_name, const char* key) {
    SKA_ASSERT(audio_source_map != NULL);
    if (ska_string_hash_map_has(audio_source_map, key)) {
        return (SkaAudioSource*)ska_string_hash_map_get(audio_source_map, key);
    }
    SkaAudioSource* new_audio_source = ska_audio_load_audio_source_wav(file_name);
    if (!new_audio_source) {
        return NULL;
    }
    ska_string_hash_map_add(audio_source_map, key, new_audio_source, sizeof(SkaAudioSource));
    return new_audio_source;
}

SkaAudioSource* ska_asset_manager_get_audio_source(const char* key) {
    return (SkaAudioSource*) ska_string_hash_map_get(audio_source_map, key);
}

bool ska_asset_manager_has_audio_source(const char* key) {
    return ska_string_hash_map_has(audio_source_map, key);
}
#endif // #if SKA_AUDIO
