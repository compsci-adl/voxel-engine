#ifndef MESH_H
#define MESH_H

// #include "smolgl.h"
#include "Camera.h"

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

template <typename VERTEX_TYPE>
class ChunkMesh {
public:
    static constexpr bool DEBUG_TRIANGLES = false; // Display green triangles on blocks
    static constexpr int MESH_VERTEX_BUFFERS = 2;
    int vertexCount;   // Number of vertices stored in arrays
    int triangleCount; // Number of triangles stored (indexed or not)

    VERTEX_TYPE *vertices;
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
    unsigned int *vboId; // OpenGL Vertex Buffer Objects id (default vertex data)
};

struct ChunkModel {
    glm::mat4 transform; // Local transform matrix
    int meshCount;       // Number of meshes
    int materialCount;   // Number of materials
    ChunkMesh<int> *meshes;   // Meshes array
    Material *materials; // Materials array
    int *meshMaterial;   // Mesh material number
};




#endif // MESH_H