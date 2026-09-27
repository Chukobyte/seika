#if SKA_RENDERING

#include "shader_cache.h"

#include <string.h>

#include "shader_file_parser.h"
#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/asset/asset_file_loader.h"
#include "seika/data_structures/id_queue.h"
#include "seika/logger.h"

// --- Parsed Shader Cache --- //
typedef struct SkaParsedShaderCacheItem {
    SkaShaderFileParseResult parse_result;
} SkaParsedShaderCacheItem;

SkaStringHashMap* parsed_shader_cache_map = NULL;

// --- Shader Cache --- //
static SkaShaderInstance* instance_cache[SKA_SHADER_INSTANCE_MAX_INSTANCES];
static SkaIdQueue* shader_instance_id_queue = NULL;

void ska_shader_cache_initialize() {
    SKA_ASSERT(shader_instance_id_queue == NULL);
    SKA_ASSERT(parsed_shader_cache_map == NULL);
    shader_instance_id_queue = ska_id_queue_create(SKA_SHADER_INSTANCE_MAX_INSTANCES);
    for (uint32_t i = 0; i < SKA_SHADER_INSTANCE_MAX_INSTANCES; i++) {
        ska_id_queue_enqueue(shader_instance_id_queue, i);
        instance_cache[i] = NULL;
    }
    parsed_shader_cache_map = ska_string_hash_map_create_default_capacity();
}

void ska_shader_cache_finalize() {
    SKA_ASSERT(shader_instance_id_queue != NULL);
    SKA_ASSERT(parsed_shader_cache_map != NULL);
    ska_id_queue_destroy(shader_instance_id_queue);
    shader_instance_id_queue = NULL;
    SKA_STRING_HASH_MAP_FOR_EACH(parsed_shader_cache_map, iter) {
        SkaParsedShaderCacheItem* cache_item = (SkaParsedShaderCacheItem*)iter.value;
        ska_shader_file_parse_clear_parse_result(&cache_item->parse_result);
    }
    ska_string_hash_map_deinit(parsed_shader_cache_map);
    parsed_shader_cache_map = NULL;
}

SkaShaderInstanceId ska_shader_cache_add_instance(SkaShaderInstance* instance) {
    SkaShaderInstanceId new_id;
    const bool succes = ska_id_queue_dequeue(shader_instance_id_queue, &new_id);
    SKA_ASSERT(succes);
    instance_cache[new_id] = instance;
    return new_id;
}

void ska_shader_cache_remove_instance(SkaShaderInstanceId instance_id) {
    instance_cache[instance_id] = NULL;
    ska_id_queue_enqueue(shader_instance_id_queue, instance_id);
}

SkaShaderInstance* ska_shader_cache_get_instance(SkaShaderInstanceId instance_id) {
    return instance_cache[instance_id];
}

SkaShaderInstance* ska_shader_cache_get_instance_checked(SkaShaderInstanceId instance_id) {
    if (instance_id != SKA_SHADER_INSTANCE_INVALID_ID) {
        return instance_cache[instance_id];
    }
    return NULL;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add(const char* shader_path) {
    if (!ska_string_hash_map_has(parsed_shader_cache_map, shader_path)) {
        char* shader_source = ska_asset_file_loader_read_file_contents_as_string_without_raw(shader_path, NULL);
        // Uncomment when needing to debug shaders
//    se_logger_debug("shader source = \n%s", shaderSource);
        if (shader_source) {
            SkaParsedShaderCacheItem new_cache_item;
            new_cache_item.parse_result = ska_shader_file_parser_parse_shader(shader_source);
            const bool has_error_message =
                strlen(new_cache_item.parse_result.error_message) > 0;
            if (has_error_message) {
              ska_logger_error("Shader parse error = '%s'\n",
                               new_cache_item.parse_result.error_message);
              return SKA_SHADER_INSTANCE_INVALID_ID;
            }
            ska_string_hash_map_add(parsed_shader_cache_map, shader_path, &new_cache_item, sizeof(SkaParsedShaderCacheItem));
        } else {
            ska_logger_error("Failed to read shader source from '%s'", shader_path);
            return SKA_SHADER_INSTANCE_INVALID_ID;
        }
    }

    SkaParsedShaderCacheItem* cache_item = (SkaParsedShaderCacheItem*)ska_string_hash_map_get(parsed_shader_cache_map, shader_path);
    SkaShader *new_shader = ska_shader_compile_new_shader(
        cache_item->parse_result.parse_data.full_vertex_source,
        cache_item->parse_result.parse_data.full_fragment_source);
    if (new_shader == NULL) {
        ska_logger_error("Error compiling shader from path = '%s'\n", shader_path);
        return SKA_SHADER_INSTANCE_INVALID_ID;
    }
    SkaShaderInstance* shader_instance = ska_shader_instance_create_from_shader(new_shader);
    for (usize i = 0; i < cache_item->parse_result.parse_data.uniform_count;
         i++) {
      ska_shader_instance_param_create_from_copy(
          shader_instance, &cache_item->parse_result.parse_data.uniforms[i]);
    }
    SkaShaderInstanceId new_id = ska_shader_cache_add_instance(shader_instance);
    return new_id;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_raw(const char* vertex_path, const char* fragment_path) {
    char* vertex_source = ska_asset_file_loader_read_file_contents_as_string(vertex_path, NULL);
    char* fragment_source = ska_asset_file_loader_read_file_contents_as_string(fragment_path, NULL);
    const SkaShaderInstanceId new_id = ska_shader_cache_create_instance_and_add_from_source(vertex_source, fragment_source);
    SKA_FREE(vertex_source);
    SKA_FREE(fragment_source);
    return new_id;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_source(const char* vertex_source, const char* fragment_source) {
    SkaShaderInstance* instance = ska_shader_instance_create(vertex_source, fragment_source);
    const SkaShaderInstanceId new_id = ska_shader_cache_add_instance(instance);
    return new_id;
}

#endif // #if SKA_RENDERING
