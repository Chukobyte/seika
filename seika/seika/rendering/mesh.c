#if SKA_RENDERING

#include <math.h>
#include <stdlib.h>

#include "mesh.h"
#include "seika/memory.h"

SkaMesh ska_mesh_create(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count) {
    return ska_mesh_create2(vertices, vertex_count, indices, index_count, NULL, NULL);
}

SkaMesh ska_mesh_create2(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count, SkaTexture* specular, SkaTexture* diffuse) {
    // SkaMesh* mesh = SKA_ALLOC(SkaMesh);
    SkaMesh mesh = {0};
    mesh.vertex_count = vertex_count;
    mesh.index_count = index_count;
    mesh.has_indices = (indices != NULL && index_count > 0);
    mesh.specular = specular;
    mesh.diffuse = diffuse;
    mesh.normal = NULL; // TODO: Implement

    // Generate VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Generate and fill VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    const usize vertex_data_size = vertex_count * 8 * sizeof(GLfloat);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertex_data_size), vertices,
                 GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, normal));
    glEnableVertexAttribArray(1);

    // UV attribute (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, tex_coords));
    glEnableVertexAttribArray(2);

    // Generate and fill EBO if we have indices
    if (mesh.has_indices) {
      glGenBuffers(1, &mesh.ebo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   (GLsizeiptr)(index_count * sizeof(uint32)), indices,
                   GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    return mesh;
}

SkaMesh ska_mesh_create_sphere(uint32 rings, uint32 sectors) {
    const uint32 vertex_count = (rings + 1) * (sectors + 1);
    const uint32 index_count = rings * sectors * 6;

    // Scratch CPU-side buffers, uploaded to the GPU by ska_mesh_create then freed -- not tracked
    // engine allocations like the mesh itself.
    f32* vertices = (f32*)malloc((usize)vertex_count * 8 * sizeof(f32));
    uint32* indices = (uint32*)malloc((usize)index_count * sizeof(uint32));

    const f32 pi = 3.14159265358979323846f;
    for (uint32 r = 0; r <= rings; r++) {
        const f32 phi = pi * (f32)r / (f32)rings;
        const f32 y = cosf(phi);
        const f32 ring_radius = sinf(phi);
        for (uint32 s = 0; s <= sectors; s++) {
            const f32 theta = 2.0f * pi * (f32)s / (f32)sectors;
            const f32 x = ring_radius * cosf(theta);
            const f32 z = ring_radius * sinf(theta);
            const uint32 base = (r * (sectors + 1) + s) * 8;
            // Unit sphere centered at the origin -- position and normal are the same vector.
            vertices[base + 0] = x;
            vertices[base + 1] = y;
            vertices[base + 2] = z;
            vertices[base + 3] = x;
            vertices[base + 4] = y;
            vertices[base + 5] = z;
            vertices[base + 6] = (f32)s / (f32)sectors;
            vertices[base + 7] = (f32)r / (f32)rings;
        }
    }

    uint32 idx = 0;
    for (uint32 r = 0; r < rings; r++) {
        for (uint32 s = 0; s < sectors; s++) {
            const uint32 a = r * (sectors + 1) + s;
            const uint32 b = a + sectors + 1;
            indices[idx++] = a;
            indices[idx++] = b;
            indices[idx++] = a + 1;
            indices[idx++] = a + 1;
            indices[idx++] = b;
            indices[idx++] = b + 1;
        }
    }

    SkaMesh mesh = ska_mesh_create(vertices, vertex_count, indices, index_count);
    free(vertices);
    free(indices);
    return mesh;
}

void ska_mesh_destroy(SkaMesh* mesh) {
    if (mesh) {
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        if (mesh->has_indices) {
          glDeleteBuffers(1, &mesh->ebo);
        }
        SKA_FREE(mesh);
    }
}

SkaModel* ska_model_create() {
    SkaModel* model = SKA_ALLOC(SkaModel);
    model->meshes_list = ska_array_list_init_default_capacity(sizeof(SkaMesh));
    // model->texturesList = ska_array_list_init_default_capacity(sizeof(SkaTexture*));
    return model;
}

void ska_model_destroy(SkaModel* model) {
    SKA_FREE(model);
}

#endif // #if SKA_RENDERING
