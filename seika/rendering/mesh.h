#pragma once

#if SKA_RENDERING

#include "texture.h"
#include "seika/data_structures/array_list.h"

// Mesh and model implementation

// Vertex data that can be safely cast to float arrays to upload to the GPU
typedef struct SkaVertex {
    SkaVector3 position;
    SkaVector3 normal;
    SkaVector2 tex_coords;
    // SkaVector3 tangent;
    // SkaVector3 bitTangent;
} SkaVertex;

_Static_assert(sizeof(SkaVertex) == 32, "SkaVertex must be tightly packed with 32 bytes");

typedef struct SkaMesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    uint32 vertex_count;
    uint32 index_count;
    bool has_indices;
    SkaTexture* specular;
    SkaTexture* diffuse;
    SkaTexture* normal;
} SkaMesh;

typedef struct SkaModel {
  SkaArrayList meshes_list;
  // SkaArrayList texturesList;
} SkaModel;

SkaMesh ska_mesh_create(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count);
SkaMesh ska_mesh_create2(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count, SkaTexture* specular, SkaTexture* diffuse);
void ska_mesh_destroy(SkaMesh* mesh);

// Generates a UV sphere of radius 1 centered at the origin (scale it via the model matrix for a
// different radius, same convention as SKA_CUBE_VERTICES below). rings/sectors control
// tessellation -- 16/16 is a reasonable default for a primitive draw call, not a hero asset.
SkaMesh ska_mesh_create_sphere(uint32 rings, uint32 sectors);

SkaModel* ska_model_create();
void ska_model_destroy(SkaModel* model);

// Standard cube vertices with positions, normals, and UVs
// Layout: [pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, uv.x, uv.y]
// 36 vertices (6 faces × 2 triangles × 3 vertices)
#define SKA_CUBE_VERTICES \
{ \
    /* Back face */ \
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, \
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, \
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, \
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, \
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, \
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, \
    \
    /* Front face */ \
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, \
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f, \
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, \
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, \
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f, \
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, \
    \
    /* Left face */ \
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, \
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, \
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, \
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, \
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, \
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, \
    \
    /* Right face */ \
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, \
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, \
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, \
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, \
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, \
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, \
    \
    /* Bottom face */ \
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, \
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f, \
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, \
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, \
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f, \
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, \
    \
    /* Top face */ \
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, \
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, \
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, \
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, \
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, \
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f  \
}

#define SKA_CUBE_VERTEX_COUNT 36

#endif // #if SKA_RENDERING
