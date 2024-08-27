#ifndef MESH_H
#define MESH_H

#include "smolgl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <stdlib.h>

#include <stdio.h>

// Material, includes shader and maps
struct Material {
    Shader* shader; // Material shader
    // MaterialMap *maps;      // Material maps array (MAX_MATERIAL_MAPS)
    // float params[4];        // Material generic parameters (if required)
    Material();
    Material(Shader* _shader);
};

Material::Material() {}

Material::Material(Shader* _shader) { shader = _shader; }

struct ChunkMesh {
    static constexpr int MESH_VERTEX_BUFFERS = 2;
    int vertexCount;   // Number of vertices stored in arrays
    int triangleCount; // Number of triangles stored (indexed or not)

    int *vertices;
    /*
            Represents vertex data by packing them into a 32-bit float:
            [start]...ttttttfffzzzzzzyyyyyyxxxxxx[end]
            where:
            - x, y, z: represent bits occupied to represent vertex position
   within a chunk
            - f: bits occupied to represent the vertex's face's normal vector
            - t: block type ID
    */
    unsigned int *indices; // Vertex indices (in case vertex data comes indexed)

    // OpenGL identifiers
    unsigned int vaoId; // OpenGL Vertex Array Object id
    unsigned int
        *vboId; // OpenGL Vertex Buffer Objects id (default vertex data)
};

struct ChunkModel {
    glm::mat4 transform; // Local transform matrix
    int meshCount;       // Number of meshes
    int materialCount;   // Number of materials
    ChunkMesh *meshes;   // Meshes array
    Material *materials; // Materials array
    int *meshMaterial;   // Mesh material number
};

// Upload vertex data into a VAO (if supported) and VBO
void UploadChunkMesh(ChunkMesh *mesh, bool dynamic) {
    // printf("Uploading Chunk Mesh...\n");
    if (mesh->vaoId > 0) {
        // Check if mesh has already been loaded in GPU
        // printf("VAO: [ID %i] Trying to re-load an already loaded mesh\n",
            //    mesh->vaoId);
        return;
    }

    mesh->vboId = (unsigned int *)calloc(ChunkMesh::MESH_VERTEX_BUFFERS,
                                         sizeof(unsigned int));

    mesh->vaoId = 0;    // Vertex Array Object
    mesh->vboId[0] = 0; // Vertex buffer: positions
    mesh->vboId[1] = 0; // Vertex buffer: indices

    glGenVertexArrays(1, &(mesh->vaoId));
    glBindVertexArray(mesh->vaoId);

    // NOTE: Vertex attributes must be uploaded considering default locations
    // points and available vertex data

    // Enable vertex data: (shader-location = 0)
    void *vertices = mesh->vertices;
    mesh->vboId[0] = smolLoadVertexBuffer(
        vertices, mesh->vertexCount * sizeof(int), dynamic);
    // TODO: we hardcode this for now...
    // smolSetVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 1,
    //                        GL_INT, 0, 1, 0);
    glVertexAttribIPointer(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 1,
                           GL_INT, sizeof(int), (void *)0);
    smolEnableVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

    if (mesh->indices != NULL) {
        // TODO: use unsigned short?
        mesh->vboId[1] = smolLoadVertexBufferElement(
            mesh->indices, mesh->triangleCount * 3 * sizeof(unsigned int),
            dynamic);
    }

    // if (mesh->vaoId > 0) TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded
    // successfully to VRAM (GPU)", mesh->vaoId); else TRACELOG(LOG_INFO, "VBO:
    // Mesh uploaded successfully to VRAM (GPU)");

    if (mesh->vaoId > 0) {
        // printf("VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)\n",
            //    mesh->vaoId);
    } else {
        // printf("VBO: Mesh uploaded successfully to VRAM (GPU)\n");
    }

    glBindVertexArray(0);
}

// Unload mesh from memory (RAM and VRAM)
void UnloadChunkMesh(ChunkMesh mesh) {
    // Unload rlgl mesh vboId data
    smolUnloadVertexArray(mesh.vaoId);

    if (mesh.vboId != NULL)
        for (int i = 0; i < ChunkMesh::MESH_VERTEX_BUFFERS; i++)
            glDeleteBuffers(1, &(mesh.vboId[i]));
    free(mesh.vboId);

    free(mesh.vertices);
    free(mesh.indices);
}

void DrawChunkMesh(ChunkMesh mesh, Material material, glm::vec3 position) {
    material.shader->use();

    glm::mat4 projection = glm::perspective(
        glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, zNear, zFar);
    material.shader->setMat4("projection", projection);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    material.shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    material.shader->setMat4("model", model);

    glBindVertexArray(mesh.vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
    glVertexAttribIPointer(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 1,
                           GL_INT, sizeof(int), (void *)0);
    smolEnableVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

    material.shader->setVec3("worldPos", position);
    // Draw mesh
    if (mesh.indices != NULL) {
        material.shader->setBool("useInColor", true);
        material.shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        smolDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
        material.shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
    }
    else {
        material.shader->setBool("useInColor", true);
        material.shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        smolDrawVertexArray(0, mesh.triangleCount * 3);
        // material.shader->setVec3("inColor", {0.0f, 0.5f, 0.0f});
        material.shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArray(0, mesh.triangleCount * 3);
    }

    // Disable all possible vertex array objects (or VBOs)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Disable shader program
    glUseProgram(0);
}

// ChunkModel LoadChunkModelFromMesh(ChunkMesh mesh, Material material) {
//   ChunkModel model = {0};

//   model.transform = MatrixIdentity();

//   model.meshCount = 1;
//   model.meshes = (ChunkMesh *)RL_CALLOC(model.meshCount, sizeof(ChunkMesh));
//   model.meshes[0] = mesh;

//   model.materialCount = 1;
//   model.materials =
//       (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
//   model.materials[0] = material;

//   model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
//   model.meshMaterial[0] = 0; // First material index

//   return model;
// }

// // Draw a model with extended parameters
// void DrawChunkModelEx(ChunkModel model, Vector3 position, Vector3
// rotationAxis,
//                       float rotationAngle, Vector3 scale, Color tint) {
//   // Calculate transformation matrix from function parameters
//   // Get transform matrix (rotation -> scale -> translation)
//   Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
//   Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
//   Matrix matTranslation = MatrixTranslate(position.x, position.y,
//   position.z);

//   Matrix matTransform =
//       MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

//   // Combine model transformation matrix (model.transform) with matrix
//   generated
//   // by function parameters (matTransform)
//   model.transform = MatrixMultiply(model.transform, matTransform);

//   for (int i = 0; i < model.meshCount; i++) {
//     Color color =
//         model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

//     Color colorTint = WHITE;
//     colorTint.r = (unsigned char)(((int)color.r * (int)tint.r) / 255);
//     colorTint.g = (unsigned char)(((int)color.g * (int)tint.g) / 255);
//     colorTint.b = (unsigned char)(((int)color.b * (int)tint.b) / 255);
//     colorTint.a = (unsigned char)(((int)color.a * (int)tint.a) / 255);

//     model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color =
//         colorTint;
//     DrawChunkMesh(model.meshes[i], model.materials[model.meshMaterial[i]],
//                   model.transform);
//     model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color =
//         color;
//   }
// }

// // Draw a model (with texture if set)
// void DrawChunkModel(ChunkModel model, Vector3 position, float scale,
//                     Color tint) {
//   Vector3 vScale = {scale, scale, scale};
//   Vector3 rotationAxis = {0.0f, 1.0f, 0.0f};

//   DrawChunkModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
// }

// void DrawChunkModelWires(ChunkModel model, Vector3 position, float scale,
//                          Color tint) {
//   rlEnableWireMode();

//   DrawChunkModel(model, position, scale, tint);

//   rlDisableWireMode();
// }

#endif // MESH_H