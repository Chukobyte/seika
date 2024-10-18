#include "shader_cache.h"

#include <string.h>

#include "shader_file_parser.h"
#include "seika/memory.h"
#include "seika/assert.h"
#include "seika/asset/asset_file_loader.h"
#include "seika/data_structures/queue.h"
#include "seika/logger.h"

// --- Parsed Shader Cache --- //
typedef struct SkaParsedShaderCacheItem {
    SkaShaderFileParseResult parseResult;
} SkaParsedShaderCacheItem;

SkaStringHashMap* parsedShaderCacheMap = NULL;

// --- Shader Cache --- //
static SkaShaderInstance* instanceCache[SKA_SHADER_INSTANCE_MAX_INSTANCES];
static SkaQueue* shaderInstanceIdQueue = NULL;

void ska_shader_cache_initialize() {
    SKA_ASSERT(shaderInstanceIdQueue == NULL);
    SKA_ASSERT(parsedShaderCacheMap == NULL);
    shaderInstanceIdQueue = ska_queue_create(SKA_SHADER_INSTANCE_MAX_INSTANCES, SKA_SHADER_INSTANCE_INVALID_ID);
    for (uint32_t i = 0; i < SKA_SHADER_INSTANCE_MAX_INSTANCES; i++) {
        ska_queue_enqueue(shaderInstanceIdQueue, i);
        instanceCache[i] = NULL;
    }
    parsedShaderCacheMap = ska_string_hash_map_create_default_capacity();
}

void ska_shader_cache_finalize() {
    SKA_ASSERT(shaderInstanceIdQueue != NULL);
    SKA_ASSERT(parsedShaderCacheMap != NULL);
    ska_queue_destroy(shaderInstanceIdQueue);
    shaderInstanceIdQueue = NULL;
    SKA_STRING_HASH_MAP_FOR_EACH(parsedShaderCacheMap, iter) {
        SkaStringHashMapNode* node = iter.pair;
        SkaParsedShaderCacheItem* cacheItem = (SkaParsedShaderCacheItem*)node->value;
        ska_shader_file_parse_clear_parse_result(&cacheItem->parseResult);
    }
    ska_string_hash_map_destroy(parsedShaderCacheMap);
    parsedShaderCacheMap = NULL;
}

SkaShaderInstanceId ska_shader_cache_add_instance(SkaShaderInstance* instance) {
    const SkaShaderInstanceId newId = ska_queue_dequeue(shaderInstanceIdQueue);
    instanceCache[newId] = instance;
    return newId;
}

void ska_shader_cache_remove_instance(SkaShaderInstanceId instanceId) {
    instanceCache[instanceId] = NULL;
    ska_queue_enqueue(shaderInstanceIdQueue, instanceId);
}

SkaShaderInstance* ska_shader_cache_get_instance(SkaShaderInstanceId instanceId) {
    return instanceCache[instanceId];
}

SkaShaderInstance* ska_shader_cache_get_instance_checked(SkaShaderInstanceId instanceId) {
    if (instanceId != SKA_SHADER_INSTANCE_INVALID_ID) {
        return instanceCache[instanceId];
    }
    return NULL;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add(const char* shaderPath) {
    if (!ska_string_hash_map_has(parsedShaderCacheMap, shaderPath)) {
        char* shaderSource = ska_asset_file_loader_read_file_contents_as_string_without_raw(shaderPath, NULL);
        // Uncomment when needing to debug shaders
//    se_logger_debug("shader source = \n%s", shaderSource);
        if (shaderSource) {
            SkaParsedShaderCacheItem newCacheItem;
            newCacheItem.parseResult = ska_shader_file_parser_parse_shader(shaderSource);
            const bool hasErrorMessage = strlen(newCacheItem.parseResult.errorMessage) > 0;
            if (hasErrorMessage) {
                ska_logger_error("Shader parse error = '%s'\n", newCacheItem.parseResult.errorMessage);
                return SKA_SHADER_INSTANCE_INVALID_ID;
            }
            ska_string_hash_map_add(parsedShaderCacheMap, shaderPath, &newCacheItem, sizeof(SkaParsedShaderCacheItem));
        } else {
            ska_logger_error("Failed to read shader source from '%s'", shaderPath);
            return SKA_SHADER_INSTANCE_INVALID_ID;
        }
    }

    SkaParsedShaderCacheItem* cacheItem = (SkaParsedShaderCacheItem*)ska_string_hash_map_get(parsedShaderCacheMap, shaderPath);
    SkaShader* newShader = ska_shader_compile_new_shader(cacheItem->parseResult.parseData.fullVertexSource, cacheItem->parseResult.parseData.fullFragmentSource);
    if (newShader == NULL) {
        ska_logger_error("Error compiling shader from path = '%s'\n", shaderPath);
        return SKA_SHADER_INSTANCE_INVALID_ID;
    }
    SkaShaderInstance* shaderInstance = ska_shader_instance_create_from_shader(newShader);
    for (usize i = 0; i < cacheItem->parseResult.parseData.uniformCount; i++) {
        ska_shader_instance_param_create_from_copy(shaderInstance, &cacheItem->parseResult.parseData.uniforms[i]);
    }
    SkaShaderInstanceId newId = ska_shader_cache_add_instance(shaderInstance);
    return newId;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_raw(const char* vertexPath, const char* fragmentPath) {
    char* vertexSource = ska_asset_file_loader_read_file_contents_as_string(vertexPath, NULL);
    char* fragmentSource = ska_asset_file_loader_read_file_contents_as_string(fragmentPath, NULL);
    const SkaShaderInstanceId newId = ska_shader_cache_create_instance_and_add_from_source(vertexSource, fragmentSource);
    SKA_MEM_FREE(vertexSource);
    SKA_MEM_FREE(fragmentSource);
    return newId;
}

SkaShaderInstanceId ska_shader_cache_create_instance_and_add_from_source(const char* vertexSource, const char* fragmentSource) {
    SkaShaderInstance* instance = ska_shader_instance_create(vertexSource, fragmentSource);
    const SkaShaderInstanceId newId = ska_shader_cache_add_instance(instance);
    return newId;
}
