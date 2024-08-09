#ifndef CHUNK_H
#define CHUNK_H
#include "Block.h"
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <string.h>

/*
    TODO LIST:
    - chunk render func inside our outside? how do we want to style our
   codebase?
    - chunk unloading?
*/

struct Chunk {
    static const int CHUNK_SIZE = 16;
    static bool debugMode;

    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    Mesh mesh;
    Model model;
    Vector3 chunkPosition; // minimum corner of the chunk

    Chunk(Vector3 position);
    ~Chunk();

    void createMesh();
    void load();
    void unload();
    void rebuildMesh();
    void setup();
    void render();
    BoundingBox getBoundingBox();
    void randomize();
    void AddCubeFace(Mesh *mesh, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4,
                     Vector3 normal, int *vCount, int *iCount, float r, float g,
                     float b, float a);
    void CreateCube(Mesh *mesh, Vector3 position, float size, int *vCount,
                    int *iCount, int blockX, int blockY, int blockZ);
    bool isLoaded();
    bool isSetup();

    inline int getIndex(int x, int y, int z) const {
        return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    }

  private:
    bool loaded;
    bool hasSetup;
};

bool Chunk::debugMode = false;

Chunk::Chunk(Vector3 position) {
    // blocks = new Block[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    chunkPosition = position;
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

    int totalVertices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 4 * 2;
    int totalIndices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6 * 2;

    float *normals = (float *)malloc(totalVertices * 3 * sizeof(float));
    float *texcoords = (float *)malloc(totalVertices * 2 * sizeof(float));
    unsigned int *indices =
        (unsigned int *)malloc(totalIndices * sizeof(unsigned int));
    unsigned char *colors =
        (unsigned char *)malloc(totalVertices * 4 * sizeof(unsigned char));

    mesh = {0};
    mesh.vertexCount = 0;
    mesh.triangleCount = 0;
    mesh.vertices = (float *)malloc(totalVertices * 3 * sizeof(float));
    mesh.normals = normals;
    mesh.texcoords = texcoords;
    mesh.indices = indices;
    mesh.colors = colors;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (!blocks[getIndex(x, y, z)].isActive) {
                    continue;
                }
                Vector3 pos = {Block::BLOCK_RENDER_SIZE * (float)x,
                               Block::BLOCK_RENDER_SIZE * (float)y,
                               Block::BLOCK_RENDER_SIZE * (float)z};
                pos = Vector3Add(pos, chunkPosition);
                CreateCube(&mesh, pos, Block::BLOCK_RENDER_SIZE,
                           &mesh.vertexCount, &indexCount, x, y, z);
            }
        }
    }

    mesh.triangleCount = indexCount / 3;
    UploadMesh(&mesh, false);
    model = LoadModelFromMesh(mesh);
}

void Chunk::load() { loaded = true; }

void Chunk::unload() {
    // UnloadModel(model);
    UnloadMesh(mesh);
    loaded = false;
    hasSetup = false;
}

void Chunk::rebuildMesh() {
    UnloadMesh(mesh);
    createMesh();
}

void Chunk::setup() {
    randomize();
    createMesh();
    hasSetup = true;
}

// renders the chunk
void Chunk::render() {
    Color color = DARKGREEN;
    DrawModel(model, {0.0, 0.0, 0.0}, 1.0f, color);
    Color wireColor = {(unsigned char)209.0f, (unsigned char)255.0f,
                       (unsigned char)189.0f, (unsigned char)255.0f};
    DrawModelWires(model, {0.0, 0.0, 0.0}, 1.0f, wireColor);
    if (Chunk::debugMode) {
        BoundingBox bBox = getBoundingBox();
        DrawBoundingBox(bBox, YELLOW);
        DrawSphere(bBox.min, 1.0f, ORANGE);
        DrawSphere(bBox.max, 1.0f, PINK);
    }
}

BoundingBox Chunk::getBoundingBox() {
    Vector3 max = {chunkPosition.x + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
                   chunkPosition.y + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
                   chunkPosition.z + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE};
    BoundingBox bBox = {chunkPosition, max};
    return bBox;
}

void Chunk::randomize() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int index = getIndex(x, y, z);
                blocks[index].isActive = (rand() % 2 == 0) ? false : true;
            }
        }
    }
}

// void deactivateBlock(Vector2 coords) {
// }

void Chunk::AddCubeFace(Mesh *mesh, Vector3 p1, Vector3 p2, Vector3 p3,
                        Vector3 p4, Vector3 normal, int *vCount, int *iCount,
                        float r, float g, float b, float a) {
    int v1 = *vCount;
    int v2 = *vCount + 1;
    int v3 = *vCount + 2;
    int v4 = *vCount + 3;

    // Add vertices
    mesh->vertices[v1 * 3] = p1.x;
    mesh->vertices[v1 * 3 + 1] = p1.y;
    mesh->vertices[v1 * 3 + 2] = p1.z;
    mesh->vertices[v2 * 3] = p2.x;
    mesh->vertices[v2 * 3 + 1] = p2.y;
    mesh->vertices[v2 * 3 + 2] = p2.z;
    mesh->vertices[v3 * 3] = p3.x;
    mesh->vertices[v3 * 3 + 1] = p3.y;
    mesh->vertices[v3 * 3 + 2] = p3.z;
    mesh->vertices[v4 * 3] = p4.x;
    mesh->vertices[v4 * 3 + 1] = p4.y;
    mesh->vertices[v4 * 3 + 2] = p4.z;

    // Add normals
    for (int i = 0; i < 4; i++) {
        mesh->normals[(*vCount + i) * 3] = normal.x;
        mesh->normals[(*vCount + i) * 3 + 1] = normal.y;
        mesh->normals[(*vCount + i) * 3 + 2] = normal.z;
    }

    // Add colors
    for (int i = 0; i < 4; i++) {
        mesh->colors[(*vCount + i) * 4] = r;
        mesh->colors[(*vCount + i) * 4 + 1] = g;
        mesh->colors[(*vCount + i) * 4 + 2] = b;
        mesh->colors[(*vCount + i) * 4 + 3] = a;
    }

    // Add texture coordinates (dummy values)
    mesh->texcoords[(*vCount) * 2] = 0.0f;
    mesh->texcoords[(*vCount) * 2 + 1] = 0.0f;
    mesh->texcoords[(*vCount + 1) * 2] = 1.0f;
    mesh->texcoords[(*vCount + 1) * 2 + 1] = 0.0f;
    mesh->texcoords[(*vCount + 2) * 2] = 1.0f;
    mesh->texcoords[(*vCount + 2) * 2 + 1] = 1.0f;
    mesh->texcoords[(*vCount + 3) * 2] = 0.0f;
    mesh->texcoords[(*vCount + 3) * 2 + 1] = 1.0f;

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

void Chunk::CreateCube(Mesh *mesh, Vector3 position, float size, int *vCount,
                       int *iCount, int blockX, int blockY, int blockZ) {
    float hs = size / 2.0f;

    Vector3 p1 = {position.x - hs, position.y - hs, position.z + hs};
    Vector3 p2 = {position.x + hs, position.y - hs, position.z + hs};
    Vector3 p3 = {position.x + hs, position.y + hs, position.z + hs};
    Vector3 p4 = {position.x - hs, position.y + hs, position.z + hs};
    Vector3 p5 = {position.x + hs, position.y - hs, position.z - hs};
    Vector3 p6 = {position.x - hs, position.y - hs, position.z - hs};
    Vector3 p7 = {position.x - hs, position.y + hs, position.z - hs};
    Vector3 p8 = {position.x + hs, position.y + hs, position.z - hs};

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

    Vector3 n1 = {0.0f, 0.0f, 1.0f};
    if (!lZPositive) {
        AddCubeFace(mesh, p1, p2, p3, p4, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }

    if (!lZNegative) {
        n1 = {0.0f, 0.0f, -1.0f};
        AddCubeFace(mesh, p5, p6, p7, p8, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }

    if (!lXPositive) {
        n1 = {1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p2, p5, p8, p3, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }

    if (!lXNegative) {
        n1 = {-1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p6, p1, p4, p7, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }

    if (!lYPositive) {
        n1 = {0.0f, 1.0f, 0.0f};
        AddCubeFace(mesh, p4, p3, p8, p7, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }

    if (!lYNegative) {
        n1 = {0.0f, -1.0f, 0.0f};
        AddCubeFace(mesh, p6, p5, p2, p1, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }
}

bool Chunk::isLoaded() { return loaded; }

bool Chunk::isSetup() { return hasSetup; }

#endif // CHUNK_H
