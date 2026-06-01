#if SKA_RENDERING

#include "mesh.h"
#include "seika/memory.h"

SkaMesh ska_mesh_create(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count) {
    return ska_mesh_create2(vertices, vertex_count, indices, index_count, NULL, NULL);
}

SkaMesh ska_mesh_create2(const f32* vertices, uint32 vertex_count, const uint32* indices, uint32 index_count, SkaTexture* specular, SkaTexture* diffuse) {
    // SkaMesh* mesh = SKA_ALLOC(SkaMesh);
    SkaMesh mesh = {0};
    mesh.vertexCount = vertex_count;
    mesh.indexCount = index_count;
    mesh.hasIndices = (indices != NULL && index_count > 0);
    mesh.specular = specular;
    mesh.diffuse = diffuse;
    mesh.normal = NULL; // TODO: Implement

    // Generate VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Generate and fill VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    const usize vertexDataSize = vertex_count * 8 * sizeof(GLfloat);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertexDataSize), vertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, normal));
    glEnableVertexAttribArray(1);

    // UV attribute (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkaVertex), (void*)offsetof(SkaVertex, texCoords));
    glEnableVertexAttribArray(2);

    // Generate and fill EBO if we have indices
    if (mesh.hasIndices) {
        glGenBuffers(1, &mesh.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(index_count * sizeof(uint32)), indices, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    return mesh;
}

void ska_mesh_destroy(SkaMesh* mesh) {
    if (mesh) {
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        if (mesh->hasIndices) {
            glDeleteBuffers(1, &mesh->ebo);
        }
        SKA_FREE(mesh);
    }
}

SkaModel* ska_model_create() {
    SkaModel* model = SKA_ALLOC(SkaModel);
    model->meshesList = ska_array_list_init_default_capacity(sizeof(SkaMesh));
    // model->texturesList = ska_array_list_init_default_capacity(sizeof(SkaTexture*));
    return model;
}

void ska_model_destroy(SkaModel* model) {
    SKA_FREE(model);
}

#endif // #if SKA_RENDERING
