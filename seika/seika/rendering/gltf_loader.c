#if SKA_RENDERING

#include "gltf_loader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "seika/assert.h"
#include "../logger.h"
#include "seika/memory.h"

// Helper to get buffer data for an accessor
static const void* get_accessor_data(const cgltf_accessor* accessor) {
  const cgltf_buffer_view *buffer_view = accessor->buffer_view;
  const uint8 *data = (const uint8 *)buffer_view->buffer->data;
  return data + buffer_view->offset + accessor->offset;
}

// Helper to load a texture from GLTF
static SkaTexture* load_gltf_texture(const cgltf_texture* gltf_texture, const char* base_path) {
    if (!gltf_texture || !gltf_texture->image) {
        return NULL;
    }

    const cgltf_image* image = gltf_texture->image;

    // Handle URI-based images (most common)
    if (image->uri) {
        // Construct full path relative to GLTF file
        char full_path[512];
        const char* last_slash = strrchr(base_path, '/');
        const char* last_backslash = strrchr(base_path, '\\');
        const char* separator = last_slash > last_backslash ? last_slash : last_backslash;

        if (separator) {
            size_t dir_len = separator - base_path + 1;
            strncpy(full_path, base_path, dir_len);
            full_path[dir_len] = '\0';
            strcat(full_path, image->uri);
        } else {
            strcpy(full_path, image->uri);
        }

        return ska_texture_create(full_path);
    }

    // Handle embedded image data
    if (image->buffer_view) {
        const cgltf_buffer_view* view = image->buffer_view;
        const uint8* data = (const uint8*)view->buffer->data + view->offset;

        return ska_texture_create_from_memory(data, view->size);
    }

    return NULL;
}

// Convert GLTF primitive to SkaMesh
static SkaMesh load_gltf_primitive(const cgltf_primitive* primitive, const char* base_path) {
    // Find vertex attributes
    const cgltf_accessor* position_accessor = NULL;
    const cgltf_accessor* normal_accessor = NULL;
    const cgltf_accessor* texcoord_accessor = NULL;

    for (size_t i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* attr = &primitive->attributes[i];

        switch (attr->type) {
            case cgltf_attribute_type_position:
                position_accessor = attr->data;
                break;
            case cgltf_attribute_type_normal:
                normal_accessor = attr->data;
                break;
            case cgltf_attribute_type_texcoord:
                if (attr->index == 0) {  // Use first UV set
                    texcoord_accessor = attr->data;
                }
                break;
            default:
                break;
        }
    }

    if (!position_accessor) {
        ska_logger_error("GLTF primitive missing position data");
        return (SkaMesh){0};
    }

    uint32 vertex_count = (uint32)position_accessor->count;

    // Allocate vertex array
    SkaVertex *vertices = SKA_ALLOC_BYTES(vertex_count * sizeof(SkaVertex));
    memset(vertices, 0, vertex_count * sizeof(SkaVertex));

    // Load positions (required)
    const f32* positions = (const f32*)get_accessor_data(position_accessor);
    for (uint32 i = 0; i < vertex_count; i++) {
      vertices[i].position.x = positions[i * 3 + 0];
      vertices[i].position.y = positions[i * 3 + 1];
      vertices[i].position.z = positions[i * 3 + 2];
    }

    // Load normals (optional)
    if (normal_accessor) {
        const f32* normals = (const f32*)get_accessor_data(normal_accessor);
        for (uint32 i = 0; i < vertex_count; i++) {
          vertices[i].normal.x = normals[i * 3 + 0];
          vertices[i].normal.y = normals[i * 3 + 1];
          vertices[i].normal.z = normals[i * 3 + 2];
        }
    }

    // Load texture coordinates (optional)
    if (texcoord_accessor) {
        const f32* texcoords = (const f32*)get_accessor_data(texcoord_accessor);
        for (uint32 i = 0; i < vertex_count; i++) {
          vertices[i].tex_coords.x = texcoords[i * 2 + 0];
          vertices[i].tex_coords.y = texcoords[i * 2 + 1];
        }
    }

    // Load indices
    uint32* indices = NULL;
    uint32 index_count = 0;

    if (primitive->indices) {
        const cgltf_accessor* index_accessor = primitive->indices;
        index_count = (uint32)index_accessor->count;
        indices = SKA_ALLOC_BYTES(index_count * sizeof(uint32));

        // GLTF supports different index types
        if (index_accessor->component_type == cgltf_component_type_r_16u) {
            const uint16* src_indices = (const uint16*)get_accessor_data(index_accessor);
            for (uint32 i = 0; i < index_count; i++) {
              indices[i] = src_indices[i];
            }
        } else if (index_accessor->component_type == cgltf_component_type_r_32u) {
            const uint32* src_indices = (const uint32*)get_accessor_data(index_accessor);
            memcpy(indices, src_indices, index_count * sizeof(uint32));
        } else if (index_accessor->component_type == cgltf_component_type_r_8u) {
            const uint8* src_indices = (const uint8*)get_accessor_data(index_accessor);
            for (uint32 i = 0; i < index_count; i++) {
              indices[i] = src_indices[i];
            }
        }
    }

    // Create mesh
    SkaMesh mesh = ska_mesh_create((const f32 *)vertices, vertex_count, indices,
                                   index_count);

    // Load textures from material
    if (primitive->material) {
        const cgltf_material* material = primitive->material;

        // Load base color (diffuse) texture
        if (material->has_pbr_metallic_roughness) {
            const cgltf_texture* base_texture = material->pbr_metallic_roughness.base_color_texture.texture;
            if (base_texture) {
                mesh.diffuse = load_gltf_texture(base_texture, base_path);
            }

            // Metallic-roughness texture (can be used as specular approximation)
            const cgltf_texture* mr_texture = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
            if (mr_texture) {
                mesh.specular = load_gltf_texture(mr_texture, base_path);
            }
        }

        // Load normal map
        if (material->normal_texture.texture) {
            mesh.normal = load_gltf_texture(material->normal_texture.texture, base_path);
        }
    }

    // Clean up temporary data
    SKA_FREE(vertices);
    SKA_FREE(indices);

    return mesh;
}

// Main loading function
SkaModel *ska_model_load_gltf(const char *gltf_path, const char *bin_path) {
  // Parse GLTF file
  cgltf_options options = {0};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, gltf_path, &data);

  if (result != cgltf_result_success) {
    ska_logger_error("Failed to parse GLTF file: %s", gltf_path);
    return NULL;
  }

  // Load buffers (actual vertex/index data)
  result = cgltf_load_buffers(&options, data, bin_path);
  // result = cgltf_load_buffers(&options, data, filepath);
  if (result != cgltf_result_success) {
    ska_logger_error("Failed to load GLTF buffers: %s", bin_path);
    cgltf_free(data);
    return NULL;
  }

  // Validate the data
  result = cgltf_validate(data);
  if (result != cgltf_result_success) {
    ska_logger_error("Warning: GLTF validation failed for %s", bin_path);
  }

  // Create model
  SkaModel *model = ska_model_create();

  // Count total primitives (meshes in our system)
  uint32 total_primitives = 0;
  for (size_t i = 0; i < data->meshes_count; i++) {
    total_primitives += (uint32)data->meshes[i].primitives_count;
  }

  if (total_primitives == 0) {
    ska_logger_error("No meshes found in GLTF file: %s", gltf_path);
    cgltf_free(data);
    ska_model_destroy(model);
    return NULL;
  }

  // Allocate mesh array
  ska_array_list_reserve(&model->meshes_list,
                         total_primitives * sizeof(SkaMesh));

  // Load all meshes
  for (size_t mesh_idx = 0; mesh_idx < data->meshes_count; mesh_idx++) {
    const cgltf_mesh *gltf_mesh = &data->meshes[mesh_idx];

    // Each GLTF mesh can have multiple primitives
    for (size_t prim_idx = 0; prim_idx < gltf_mesh->primitives_count;
         prim_idx++) {
      const cgltf_primitive *primitive = &gltf_mesh->primitives[prim_idx];

      // Only load triangle meshes
      if (primitive->type != cgltf_primitive_type_triangles) {
        ska_logger_error("Skipping non-triangle primitive in mesh %zu",
                         mesh_idx);
        continue;
      }

      SkaMesh mesh = load_gltf_primitive(primitive, gltf_path);
      const bool is_mesh_valid = mesh.vertex_count > 0;
      if (is_mesh_valid) {
        ska_array_list_append(&model->meshes_list, &mesh);
      }
    }
  }

  // Clean up cgltf data
  cgltf_free(data);

  ska_logger_debug("Loaded GLTF model: %s (%u meshes)", gltf_path,
                   model->meshes_list.size);

  return model;
}

SkaModel *ska_gltf_load_model(const char *gltf_path, const char *bin_path) {
  return ska_model_load_gltf(gltf_path, bin_path);
}

#endif // #if SKA_RENDERING
