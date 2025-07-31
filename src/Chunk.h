#ifndef CHUNK_H
#define CHUNK_H
#include "Block.h"
#include "ChunkMesh.h"
#include "TerrainGenerator.h"
#include "Renderer.h"
#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>

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

    Block blocks[CHUNK_SIZE_CUBED] = {Block()};
    ChunkMesh<int> mesh;
    // ChunkModel model;
    glm::vec3 chunkPosition; // minimum corner of the chunk
    Material material;
    Renderer<int> *renderer; // reference to renderer

    Chunk(glm::vec3 position, Shader *shader, Renderer<int> * renderer);
    ~Chunk();

    void createMesh();
    void load();
    void unload();
    void rebuildMesh();
    void setup(TerrainGenerator *generator);
    void render(Camera camera);
    // BoundingBox getBoundingBox();
    void initialize(TerrainGenerator *generator);
    void AddCubeFace(ChunkMesh<int> *mesh, int p1, int p2, int p3, int p4,
                     int *vCount, int *iCount);
    void CreateCube(ChunkMesh<int> *mesh, int blockX, int blockY, int blockZ,
                    float size, int *vCount, int *iCount);
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

Chunk::Chunk(glm::vec3 position, Shader *shader, Renderer<int> * _renderer) {
    // blocks = new Block[CHUNK_SIZE_CUBED];
    chunkPosition = position;
    // material = LoadMaterialDefault();
    material = Material(shader);
    // material.maps[MATERIAL_MAP_DIFFUSE].color.a = 255.0f;
    renderer = _renderer;

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

    // set up vao and vbo
    renderer->upload(&mesh.vaoId, &mesh.vboId, mesh.vertices,
        mesh.vertexCount, mesh.indices, indexCount, false);
    // model = LoadChunkModelFromMesh(mesh, material);
    // model = LoadModelFromMesh(mesh);
}

void Chunk::load() { loaded = true; }

void Chunk::unload() {
    // UnloadModel(model);
    
    // free allocated memory
    renderer->unload(&mesh.vaoId, &mesh.vboId[0], mesh.vertices, mesh.indices);
    
    loaded = false;
    hasSetup = false;
}

void Chunk::rebuildMesh() {
    renderer->unload(&mesh.vaoId, &mesh.vboId[0], mesh.vertices, mesh.indices);
    createMesh();
}

void Chunk::setup(TerrainGenerator *generator) {
    initialize(generator);
    createMesh();
    hasSetup = true;
}

// renders the chunk
void Chunk::render(Camera camera) { 
    renderer->draw(camera, &mesh.vaoId, &mesh.vboId[0],
        mesh.vertices, mesh.vertexCount, mesh.indices, mesh.triangleCount * 3, 
        material.shader, chunkPosition);
}

// BoundingBox Chunk::getBoundingBox() {
//     glm::vec3 max = {chunkPosition.x + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
//                      chunkPosition.y + CHUNK_SIZE * Block::BLOCK_RENDER_SIZE,
//                      chunkPosition.z + CHUNK_SIZE *
//                      Block::BLOCK_RENDER_SIZE};
//     BoundingBox bBox = {chunkPosition, max};
//     return bBox;
// }

// TODO: use a terrain generator 

void Chunk::initialize(TerrainGenerator *generator) {
    // normalise chunk position from real world position to 
    // index in grid position for perlin noise based generation

    // divide by block render size
    generator->generateChunk(glm::vec3{
        chunkPosition.x / Block::BLOCK_RENDER_SIZE,
        chunkPosition.y / Block::BLOCK_RENDER_SIZE,
        chunkPosition.z / Block::BLOCK_RENDER_SIZE
    }, blocks);
}

// void deactivateBlock(Vector2 coords) {
// }

void Chunk::AddCubeFace(ChunkMesh<int> *mesh, int p1, int p2, int p3, int p4,
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

void Chunk::CreateCube(ChunkMesh<int> *mesh, int blockX, int blockY, int blockZ,
                       float size, int *vCount, int *iCount) {
    int hs = (int)(size / 2.0f);

    // TODO: casts here?
    // TODO: ignore normals for now


    // DEFINES THE 8 POINTS OF THE CUBE
    // NEED TO CHANGE BLOCKTYPE to x and y coords for map

    BlockType blockType = blocks[getIndex(blockX, blockY, blockZ)].blockType;
    // packvertex(int x, int y, int z, int normal, int tex_x, int tex_y)
    // set texture coordinates to 0,0 for now, change according to face
    int p1 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX - hs,
                                Block::BLOCK_RENDER_SIZE * blockY - hs,
                                Block::BLOCK_RENDER_SIZE * blockZ + hs, 1,
                                0, 0);
    int p2 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs, 1,
                               0, 0);
    int p3 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs, 1,
                               0, 0);
    int p4 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs, 1,
                               0, 0);
    int p5 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs, 1,
                               0, 0);
    int p6 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs, 1,
                               0, 0);
    int p7 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs, 1,
                               0, 0);
    int p8 = ChunkRenderer::packVertex(Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs, 1,
                               0, 0);

    
    // CHECKS FOR NEIGHBORING BLOCKS

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


    // ADD TRIANGLES INTO MESH
    // prevent segfault if block does not exist
    if(textureCoordMap.count(blockType) == 0) {
        std::cerr << "Block type " << blockType << " not found in textureCoordMap." << std::endl;
        exit(1);
    }

    std::vector<std::pair<int, int>> textureCoords = textureCoordMap[blockType];
    // front, back, left, right, top, bottom

    glm::vec3 n1;       // for normal??, not used. 
    // front face
    if (!lZPositive) {
        n1 = {0.0f, 0.0f, 1.0f};
        p1 = ChunkRenderer::updatePackedVertex(p1, 0, textureCoords[0].first, textureCoords[0].second + 1);
        p2 = ChunkRenderer::updatePackedVertex(p2, 0, textureCoords[0].first + 1, textureCoords[0].second + 1);
        p3 = ChunkRenderer::updatePackedVertex(p3, 0, textureCoords[0].first + 1, textureCoords[0].second);
        p4 = ChunkRenderer::updatePackedVertex(p4, 0, textureCoords[0].first, textureCoords[0].second);
        
        AddCubeFace(mesh, p1, p2, p3, p4, vCount, iCount);
    }

    // back face
    if (!lZNegative) {
        n1 = {0.0f, 0.0f, -1.0f};
        p5 = ChunkRenderer::updatePackedVertex(p5, 1, textureCoords[1].first, textureCoords[1].second + 1);
        p6 = ChunkRenderer::updatePackedVertex(p6, 1, textureCoords[1].first + 1, textureCoords[1].second + 1);
        p7 = ChunkRenderer::updatePackedVertex(p7, 1, textureCoords[1].first + 1, textureCoords[1].second);
        p8 = ChunkRenderer::updatePackedVertex(p8, 1, textureCoords[1].first, textureCoords[1].second);
        AddCubeFace(mesh, p5, p6, p7, p8, vCount, iCount);
    }

    // left face
    if (!lXPositive) {
        n1 = {1.0f, 0.0f, 0.0f};
        p2 = ChunkRenderer::updatePackedVertex(p2, 2, textureCoords[2].first, textureCoords[2].second + 1);
        p5 = ChunkRenderer::updatePackedVertex(p5, 2, textureCoords[2].first + 1, textureCoords[2].second + 1);
        p8 = ChunkRenderer::updatePackedVertex(p8, 2, textureCoords[2].first + 1, textureCoords[2].second);
        p3 = ChunkRenderer::updatePackedVertex(p3, 2, textureCoords[2].first, textureCoords[2].second);
        AddCubeFace(mesh, p2, p5, p8, p3, vCount, iCount);
    }

    // right face
    if (!lXNegative) {
        n1 = {-1.0f, 0.0f, 0.0f};
        p6 = ChunkRenderer::updatePackedVertex(p6, 3, textureCoords[3].first, textureCoords[3].second + 1);
        p1 = ChunkRenderer::updatePackedVertex(p1, 3, textureCoords[3].first + 1, textureCoords[3].second + 1);
        p4 = ChunkRenderer::updatePackedVertex(p4, 3, textureCoords[3].first + 1, textureCoords[3].second);
        p7 = ChunkRenderer::updatePackedVertex(p7, 3, textureCoords[3].first, textureCoords[3].second);
        AddCubeFace(mesh, p6, p1, p4, p7, vCount, iCount);
    }

    // top face
    if (!lYPositive) {
        n1 = {0.0f, 1.0f, 0.0f};
        p4 = ChunkRenderer::updatePackedVertex(p4, 4, textureCoords[4].first, textureCoords[4].second);
        p3 = ChunkRenderer::updatePackedVertex(p3, 4, textureCoords[4].first + 1, textureCoords[4].second);
        p8 = ChunkRenderer::updatePackedVertex(p8, 4, textureCoords[4].first + 1, textureCoords[4].second + 1);
        p7 = ChunkRenderer::updatePackedVertex(p7, 4, textureCoords[4].first, textureCoords[4].second + 1);
        AddCubeFace(mesh, p4, p3, p8, p7, vCount, iCount);
    }

    // bottom face
    if (!lYNegative) {
        n1 = {0.0f, -1.0f, 0.0f};
        p6 = ChunkRenderer::updatePackedVertex(p6, 5, textureCoords[5].first, textureCoords[5].second);
        p5 = ChunkRenderer::updatePackedVertex(p5, 5, textureCoords[5].first + 1, textureCoords[5].second);
        p2 = ChunkRenderer::updatePackedVertex(p2, 5, textureCoords[5].first + 1, textureCoords[5].second + 1);
        p1 = ChunkRenderer::updatePackedVertex(p1, 5, textureCoords[5].first, textureCoords[5].second + 1);
        AddCubeFace(mesh, p6, p5, p2, p1, vCount, iCount);
    }
}

bool Chunk::isLoaded() { return loaded; }

bool Chunk::isSetup() { return hasSetup; }

#endif // CHUNK_H
