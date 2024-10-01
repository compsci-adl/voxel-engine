#pragma once

#include "ChunkMesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Mesh {
    static constexpr unsigned int MESH_VERTEX_BUFFERS = 9;
    glm::vec3 *vertices;
    int sizeVertices;
    int numTriangles;
    unsigned int *indices;
    int sizeIndices;
    glm::vec4 color;
    unsigned int vaoId;
    unsigned int *vboId;
    // TODO: add texture
};

void UploadMesh(Mesh *mesh, bool dynamic) {
    if (mesh->vaoId > 0) {
        // Check if mesh has already been loaded in GPU
        // printf("VAO: [ID %i] Trying to re-load an already loaded mesh\n",
        //    mesh->vaoId);
        return;
    }

    mesh->vboId =
        (unsigned int *)calloc(Mesh::MESH_VERTEX_BUFFERS, sizeof(unsigned int));

    mesh->vaoId = 0;    // Vertex Array Object
    mesh->vboId[0] = 0; // Vertex buffer: positions
    mesh->vboId[1] = 0; // Vertex buffer: indices

    glGenVertexArrays(1, &(mesh->vaoId));
    glBindVertexArray(mesh->vaoId);

    // enable vertex data:
    void *vertices = mesh->vertices;
    mesh->vboId[0] = smolLoadVertexBuffer(
        vertices, mesh->sizeVertices * sizeof(glm::vec3), dynamic);
    glVertexAttribPointer(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    smolEnableVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

    if (mesh->indices != NULL) {
        // TODO: use unsigned short?
        mesh->vboId[1] = smolLoadVertexBufferElement(
            mesh->indices, mesh->sizeIndices * sizeof(unsigned int),
            dynamic);
    }

    if (mesh->vaoId > 0) {
        printf("VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)\n",
               mesh->vaoId);
    } else {
        printf("VBO: Mesh uploaded successfully to VRAM (GPU)\n");
    }

    glBindVertexArray(0);
}

// Unload mesh from memory (RAM and VRAM)
void UnloadChunkMesh(Mesh mesh) {
    // Unload rlgl mesh vboId data
    smolUnloadVertexArray(mesh.vaoId);

    if (mesh.vboId != NULL)
        for (int i = 0; i < ChunkMesh::MESH_VERTEX_BUFFERS; i++)
            glDeleteBuffers(1, &(mesh.vboId[i]));
    free(mesh.vboId);

    free(mesh.vertices);
    free(mesh.indices);
}

void DrawMesh(Camera camera, Mesh mesh, Material material, glm::vec3 position) {
    material.shader->use();

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.fov), (float)SCR_WIDTH / SCR_HEIGHT, camera.zNear, camera.zFar);
    material.shader->setMat4("projection", projection);

    glm::mat4 view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
    material.shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    material.shader->setMat4("model", model);

    glBindVertexArray(mesh.vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
    glVertexAttribPointer(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    smolEnableVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

    material.shader->setVec3("worldPos", position);
    // Draw mesh
    if (mesh.indices != NULL) {
        material.shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        smolDrawVertexArrayElements(0, mesh.numTriangles * 3, 0);
        material.shader->setVec4("inColor", mesh.color);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArrayElements(0, mesh.numTriangles * 3, 0);
    }

    // Disable all possible vertex array objects (or VBOs)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Disable shader program
    glUseProgram(0);
}