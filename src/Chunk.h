#ifndef CHUNK_H
#define CHUNK_H
#include "Block.h"
#include "ChunkMesh.h"
#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>
#include <stdlib.h>
#include <string.h>

/*
        TODO LIST:
        - chunk render func inside our outside? how do we want to style our
   codebase?
        - chunk unloading?
*/

typedef struct VoxelPoint3D {
    int x;
    int y;
    int z;
} VoxelPoint3d;

struct Chunk {
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_SIZE_CUBED =
        CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    static bool debugMode;

    Block blocks[CHUNK_SIZE_CUBED];
    ChunkMesh mesh;
    // ChunkModel model;
    glm::vec3 chunkPosition; // minimum corner of the chunk
    Material material;

    Chunk(glm::vec3 position, Shader shader);
    ~Chunk();

    void createMesh();
    void load();
    void unload();
    void rebuildMesh();
    void setup();
    void render();
    // BoundingBox getBoundingBox();
    void randomize();
    void AddCubeFace(ChunkMesh *mesh, int p1, int p2, int p3, int p4,
                     int *vCount, int *iCount);
    void CreateCube(ChunkMesh *mesh, int blockX, int blockY, int blockZ,
                    float size, int *vCount, int *iCount);
    bool isLoaded();
    bool isSetup();

    inline int getIndex(int x, int y, int z) const {
        return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    }

    // inline int packVertex(int x, int y, int z, int normal,
    //                       BlockType blockType) const {
    //     int data = 0;
    //     data |= (blockType & 127) << 21;
    //     data |= (normal & 7) << 18;
    //     data |= (z & 63) << 12;
    //     data |= (y & 63) << 6;
    //     data |= (x & 63);
    // }
    static inline int packVertex(int x, int y, int z, int normal, int type) {
        int offset = 16; // Offset to handle negative values
        return ((x + offset) & 0x3F) | (((y + offset) & 0x3F) << 6) |
               (((z + offset) & 0x3F) << 12) | ((normal & 0x7) << 18) |
               ((type & 0x7FF) << 21);
    }

  private:
    bool loaded;
    bool hasSetup;
};

bool Chunk::debugMode = false;

Chunk::Chunk(glm::vec3 position, Shader shader) {
    // blocks = new Block[CHUNK_SIZE_CUBED];
    chunkPosition = position;
    // material = LoadMaterialDefault();
    material = Material(shader);
    // material.maps[MATERIAL_MAP_DIFFUSE].color.a = 255.0f;

    hasSetup = false;
    loaded = false;
};

Chunk::~Chunk(){
    // delete blocks;
};

// create vbo to be used to render chunk
void Chunk::createMesh() {
    int vertexCount = 0;
    int indexCount = 0;

    int totalVertices = CHUNK_SIZE_CUBED * 6 * 4 * 2;
    int totalIndices = CHUNK_SIZE_CUBED * 6 * 6 * 2;

    unsigned int *indices =
        (unsigned int *)malloc(totalIndices * sizeof(unsigned int));

    mesh = {0};
    mesh.vertexCount = 0;
    mesh.triangleCount = 0;
    mesh.vertices = (int *)malloc(totalVertices * sizeof(int));
    mesh.indices = indices;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block &block = blocks[getIndex(x, y, z)];
                if (!block.isActive) {
                    continue;
                }
                CreateCube(&mesh, x, y, z, Block::BLOCK_RENDER_SIZE,
                           &mesh.vertexCount, &indexCount);
            }
        }
    }

    mesh.triangleCount = indexCount / 3;
    UploadChunkMesh(&mesh, false);
    // model = LoadChunkModelFromMesh(mesh, material);
    // model = LoadModelFromMesh(mesh);
}

void Chunk::load() { loaded = true; }

void Chunk::unload() {
    // UnloadModel(model);
    UnloadChunkMesh(mesh);
    loaded = false;
    hasSetup = false;
}

void Chunk::rebuildMesh() {
    UnloadChunkMesh(mesh);
    createMesh();
}

void Chunk::setup() {
    randomize();
    createMesh();
    hasSetup = true;
}

// renders the chunk
void Chunk::render() {
    DrawChunkMesh(mesh, material, chunkPosition);
}

// BoundingBox Chunk::getBoundingBox() {
//     glm::vec3 max = {chunkPosition.x + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
//                      chunkPosition.y + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
//                      chunkPosition.z + CHUNK_SIZE *
//                      Block::BLOCK_RENDER_SIZE};
//     BoundingBox bBox = {chunkPosition, max};
//     return bBox;
// }

void Chunk::randomize() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int index = getIndex(x, y, z);
                blocks[index].isActive = (rand() % 2 == 0) ? false : true;
                // blocks[index].isActive = true;
            }
        }
    }
}

// void deactivateBlock(Vector2 coords) {
// }

void Chunk::AddCubeFace(ChunkMesh *mesh, int p1, int p2, int p3, int p4,
                        int *vCount, int *iCount) {
    int v1 = *vCount;
    int v2 = *vCount + 1;
    int v3 = *vCount + 2;
    int v4 = *vCount + 3;

    // Add vertices
    mesh->vertices[v1] = p1;
    mesh->vertices[v2] = p2;
    mesh->vertices[v3] = p3;
    mesh->vertices[v4] = p4;

    // Add indices
    mesh->indices[*iCount] = v1;
    mesh->indices[*iCount + 1] = v2;
    mesh->indices[*iCount + 2] = v3;
    mesh->indices[*iCount + 3] = v1;
    mesh->indices[*iCount + 4] = v3;
    mesh->indices[*iCount + 5] = v4;

    *vCount += 4;
    *iCount += 6;
}

void Chunk::CreateCube(ChunkMesh *mesh, int blockX, int blockY, int blockZ,
                       float size, int *vCount, int *iCount) {
    float hs = size / 2.0f;

    // TODO: casts here?
    // TODO: ignore normals for now
    int p1 = Chunk::packVertex(blockX - hs, blockY - hs, blockZ + hs, 1,
                               BlockType::Grass);
    int p2 = Chunk::packVertex(blockX + hs, blockY - hs, blockZ + hs, 1,
                               BlockType::Grass);
    int p3 = Chunk::packVertex(blockX + hs, blockY + hs, blockZ + hs, 1,
                               BlockType::Grass);
    int p4 = Chunk::packVertex(blockX - hs, blockY + hs, blockZ + hs, 1,
                               BlockType::Grass);
    int p5 = Chunk::packVertex(blockX + hs, blockY - hs, blockZ - hs, 1,
                               BlockType::Grass);
    int p6 = Chunk::packVertex(blockX - hs, blockY - hs, blockZ - hs, 1,
                               BlockType::Grass);
    int p7 = Chunk::packVertex(blockX - hs, blockY + hs, blockZ - hs, 1,
                               BlockType::Grass);
    int p8 = Chunk::packVertex(blockX + hs, blockY + hs, blockZ - hs, 1,
                               BlockType::Grass);

    bool lDefault = false;
    bool lXNegative = lDefault;
    if (blockX > 0)
        lXNegative = blocks[getIndex(blockX - 1, blockY, blockZ)].isActive;
    bool lXPositive = lDefault;
    if (blockX < CHUNK_SIZE - 1)
        lXPositive = blocks[getIndex(blockX + 1, blockY, blockZ)].isActive;
    bool lYNegative = lDefault;
    if (blockY > 0)
        lYNegative = blocks[getIndex(blockX, blockY - 1, blockZ)].isActive;
    bool lYPositive = lDefault;
    if (blockY < CHUNK_SIZE - 1)
        lYPositive = blocks[getIndex(blockX, blockY + 1, blockZ)].isActive;
    bool lZNegative = lDefault;
    if (blockZ > 0)
        lZNegative = blocks[getIndex(blockX, blockY, blockZ - 1)].isActive;
    bool lZPositive = lDefault;
    if (blockZ < CHUNK_SIZE - 1)
        lZPositive = blocks[getIndex(blockX, blockY, blockZ + 1)].isActive;

    glm::vec3 n1 = {0.0f, 0.0f, 1.0f};
    if (!lZPositive) {
        AddCubeFace(mesh, p1, p2, p3, p4, vCount, iCount);
    }

    if (!lZNegative) {
        n1 = {0.0f, 0.0f, -1.0f};
        AddCubeFace(mesh, p5, p6, p7, p8, vCount, iCount);
    }

    if (!lXPositive) {
        n1 = {1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p2, p5, p8, p3, vCount, iCount);
    }

    if (!lXNegative) {
        n1 = {-1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p6, p1, p4, p7, vCount, iCount);
    }

    if (!lYPositive) {
        n1 = {0.0f, 1.0f, 0.0f};
        AddCubeFace(mesh, p4, p3, p8, p7, vCount, iCount);
    }

    if (!lYNegative) {
        n1 = {0.0f, -1.0f, 0.0f};
        AddCubeFace(mesh, p6, p5, p2, p1, vCount, iCount);
    }
}

bool Chunk::isLoaded() { return loaded; }

bool Chunk::isSetup() { return hasSetup; }

#endif // CHUNK_H
