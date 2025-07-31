#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "ChunkMesh.h"
#include "smolgl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <stdlib.h>
#include <iostream>

#include <stdio.h>


template <typename VERTEX_TYPE>
class Renderer {
protected:
    // internal functions
    void setShaderVariables(Shader *shader, Camera camera);
public:
    // set buffers
    virtual void upload(unsigned int * vaoId, unsigned int ** vboId, 
        VERTEX_TYPE * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, bool dynamic) = 0;

    // update existing buffer triangles without recreating (better for moving blocks not chunks)
    virtual void update(unsigned int * vaoId, unsigned int * vboId,
        VERTEX_TYPE * vertices, int vertexCount, unsigned int * indices, 
        int indexCount);

    // unload vaoid and free memory
    virtual void unload(unsigned int * vaoId, unsigned int * vboId, 
        VERTEX_TYPE * vertices, unsigned int * indices);

    // draw the mesh
    virtual void draw(Camera camera, unsigned int * vaoId, unsigned int * vboId,
        VERTEX_TYPE * vertices, int vertexCount, unsigned int * indices, int indexCount, 
        Shader * shader, glm::vec3 position) = 0;
    
};


class ChunkRenderer : public Renderer<int> {
public:
    void upload(unsigned int * vaoId, unsigned int ** vboId, 
        int * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, bool dynamic) override;

    void draw(Camera camera, unsigned int * vaoId, unsigned int * vboId,
        int * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, Shader * shader, glm::vec3 position) override;

    static int packVertex(int x, int y, int z, int normal, int u, int v);

    static int updatePackedVertex(int packedVertex, int normal, int u, int v);
};


class BlockRenderer : public Renderer<float> {
public:
    void upload(unsigned int * vaoId, unsigned int ** vboId, 
        float * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, bool dynamic) override;

    void draw(Camera camera, unsigned int * vaoId, unsigned int * vboId,
        float * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, Shader * shader, glm::vec3 position) override;
};



// Function implementations
// --------------------------------------



template <typename VERTEX_TYPE>
void Renderer<VERTEX_TYPE>::setShaderVariables(Shader *shader, Camera camera) {
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.fov), (float)SCR_WIDTH / SCR_HEIGHT, camera.zNear, camera.zFar);
    shader->setMat4("projection", projection);

    glm::mat4 view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
    shader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
}



// Unload mesh from memory (RAM and VRAM)
template <typename VERTEX_TYPE>
void Renderer<VERTEX_TYPE>::unload(unsigned int * vaoId, unsigned int * vboId, 
        VERTEX_TYPE * vertices, unsigned int * indices) {
    // Unload rlgl mesh vboId data
    smolUnloadVertexArray(*vaoId);

    if (vboId != NULL)
        for (int i = 0; i < ChunkMesh<int>::MESH_VERTEX_BUFFERS; i++)
            glDeleteBuffers(1, &(vboId[i]));
    free(vboId);

    free(vertices);
    free(indices);
}


template <typename VERTEX_TYPE>
void Renderer<VERTEX_TYPE>::update(unsigned int * vaoId, unsigned int * vboId,
        VERTEX_TYPE * vertices, int vertexCount, unsigned int * indices, 
        int indexCount) {
    if (*vaoId == 0 || vboId == nullptr) {
        // printf("Error: Cannot update buffer - VAO or VBO not initialized\n");
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VERTEX_TYPE), vertices, GL_STREAM_DRAW);

    if (indices) {
        // Bind index buffer while VAO is still bound
        glBindVertexArray(*vaoId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STREAM_DRAW);
        glBindVertexArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// Upload vertex data into a VAO (if supported) and VBO
void ChunkRenderer::upload(unsigned int * vaoId, unsigned int ** vboId, 
        int * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, bool dynamic) {
    // printf("Uploading Chunk Mesh...\n");
    if (*vaoId > 0) {
        // Check if mesh has already been loaded in GPU
        // printf("VAO: [ID %i] Trying to re-load an already loaded mesh\n",
            //    mesh->vaoId);
        return;
    }

    *vboId = (unsigned int *)calloc(ChunkMesh<int>::MESH_VERTEX_BUFFERS,
                                         sizeof(unsigned int));

    *vaoId = 0;    // Vertex Array Object
    (*vboId)[0] = 0; // Vertex buffer: positions
    (*vboId)[1] = 0; // Vertex buffer: indices

    glGenVertexArrays(1, vaoId);
    glBindVertexArray(*vaoId);

    // NOTE: Vertex attributes must be uploaded considering default locations
    // points and available vertex data

    // Enable vertex data: (shader-location = 0)
    
    (*vboId)[0] = smolLoadVertexBuffer(
        vertices, vertexCount * sizeof(int), dynamic);
    // TODO: we hardcode this for now...
    // smolSetVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 1,
    //                        GL_INT, 0, 1, 0);
    glVertexAttribIPointer(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 1,
                           GL_INT, sizeof(int), (void *)0);
    smolEnableVertexAttribute(SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

    if (indices != NULL) {
        // TODO: use unsigned short?
        (*vboId)[1] = smolLoadVertexBufferElement(
            indices, indexCount * sizeof(unsigned int),
            dynamic);
    }

    // if (mesh->vaoId > 0) TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded
    // successfully to VRAM (GPU)", mesh->vaoId); else TRACELOG(LOG_INFO, "VBO:
    // Mesh uploaded successfully to VRAM (GPU)");

    if (*vaoId > 0) {
        // printf("VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)\n",
            //    mesh->vaoId);
    } else {
        // printf("VBO: Mesh uploaded successfully to VRAM (GPU)\n");
    }

    glBindVertexArray(0);
}



void ChunkRenderer::draw(Camera camera, unsigned int * vaoId, unsigned int * vboId,
        int * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, Shader * shader, glm::vec3 position) {

    shader->use();

    setShaderVariables(shader, camera);

    glBindVertexArray(*vaoId);

    shader->setVec3("worldPos", position);
    // Draw mesh
    if (indices != NULL) {
        if(ChunkMesh<int>::DEBUG_TRIANGLES){
            shader->setBool("useInColor", true);
            shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            smolDrawVertexArrayElements(0, indexCount, 0);
        }
        shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArrayElements(0, indexCount, 0);
    } else {
        if(ChunkMesh<int>::DEBUG_TRIANGLES){
            shader->setBool("useInColor", true);
            shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            smolDrawVertexArray(0, indexCount);
        }
        // shader->setVec3("inColor", {0.0f, 0.5f, 0.0f});
        shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArray(0, indexCount);
    }

    // Disable all possible vertex array objects (or VBOs)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Disable shader program
    glUseProgram(0);

}



// u/v represent texture coordinate / size
// normal is just face number, 0-5
// x, y, z are 0-63, SUPPORTS 63x63 TEXTURES, due to coordinate system
int ChunkRenderer::packVertex(int x, int y, int z, int normal, int u, int v) {
    int offset = 16; // Offset to handle negative values
    return ((x + offset) & 0x3F) |              // 6 bits for x
            (((y + offset) & 0x3F) << 6) |       // 6 bits for y
            (((z + offset) & 0x3F) << 12) |      // 6 bits for z
            ((normal & 0x7) << 18) |             // 3 bits for normal
            ((u & 0x1F) << 21) |               // 5 bits for u (x)
            ((v & 0x1F) << 26);                // 5 bits for v (y)
}



// update just the texture coordinates while maintaining the same vertex 
int ChunkRenderer::updatePackedVertex(int packedVertex, int normal, int u, int v) {
    // Clear bits 18–26 (normal, u/texX and v/texY)
    int cleared = packedVertex & ~(0x7 << 18) & ~(0x1F << 21) & ~(0x1F << 26);

    // Set new 
    cleared |= (normal & 0x7) << 18;
    cleared |= (u & 0x1F) << 21;
    cleared |= (v & 0x1F) << 26;

    return cleared;
}




void BlockRenderer::upload(unsigned int * vaoId, unsigned int ** vboId, 
        float * vertices, int vertexCount, unsigned int * indices, 
        int indexCount, bool dynamic) {
    // printf("Uploading Block Mesh...\n");
    if (*vaoId > 0) {
        // Check if mesh has already been loaded in GPU
        // printf("VAO: [ID %i] Trying to re-load an already loaded mesh\n",
            //    mesh->vaoId);
        return;
    }


    *vboId = (unsigned int *)calloc(ChunkMesh<int>::MESH_VERTEX_BUFFERS,
                                         sizeof(unsigned int));

    *vaoId = 0;    // Vertex Array Object
    (*vboId)[0] = 0; // Vertex buffer: positions
    (*vboId)[1] = 0; // Vertex buffer: indices

    glGenVertexArrays(1, vaoId);
    glBindVertexArray(*vaoId);

    // Enable vertex data: (shader-location = 0)

    
    (*vboId)[0] = smolLoadVertexBuffer(
        vertices, vertexCount * sizeof(float), dynamic);
    
    // define vertex attribute pointers - input layout for shaders
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	smolEnableVertexAttribute(0);
	// for colors - layer 1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	smolEnableVertexAttribute(1);
	// for brightness - layer 2, 1 number
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	smolEnableVertexAttribute(2);

    if (indices != NULL) {
        // TODO: use unsigned short?
        (*vboId)[1] = smolLoadVertexBufferElement(
            indices, indexCount * sizeof(unsigned int),
            dynamic);
    }

    glBindVertexArray(0);
}



void BlockRenderer::draw(Camera camera, unsigned int * vaoId, unsigned int * vboId,
        float * vertices, int vertexCount, unsigned int * indices, int indexCount, 
        Shader * shader, glm::vec3 position) {


    shader->use();

    setShaderVariables(shader, camera);

    glBindVertexArray(*vaoId);



    if(indices != NULL){
        if(ChunkMesh<int>::DEBUG_TRIANGLES){
            shader->setBool("useInColor", true);
            shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            smolDrawVertexArrayElements(0, indexCount, 0);
        }
        shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArrayElements(0, indexCount, 0);
    }
    else {
        if(ChunkMesh<int>::DEBUG_TRIANGLES){
            shader->setBool("useInColor", true);
            shader->setVec3("inColor", {0.5f, 1.0f, 0.5f});
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            smolDrawVertexArray(0, indexCount);
        }
        // shader->setVec3("inColor", {0.0f, 0.5f, 0.0f});
        shader->setBool("useInColor", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        smolDrawVertexArray(0, indexCount);
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



 // inline int packVertex(int x, int y, int z, int normal,
    //                       BlockType blockType) const {
    //     int data = 0;
    //     data |= (blockType & 127) << 21;
    //     data |= (normal & 7) << 18;
    //     data |= (z & 63) << 12;
    //     data |= (y & 63) << 6;
    //     data |= (x & 63);
    // }
    // static inline int packVertex(int x, int y, int z, int normal, int type) {
    //     int offset = 16; // Offset to handle negative values
    //     return ((x + offset) & 0x3F) | (((y + offset) & 0x3F) << 6) |
    //            (((z + offset) & 0x3F) << 12) | ((normal & 0x7) << 18) |
    //            ((type & 0x7FF) << 21);
    // }   
#endif