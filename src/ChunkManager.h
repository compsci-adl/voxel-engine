#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include "Chunk.h"
#include "raylib.h"
#include <raymath.h>
#include <unordered_map>
#include <vector>
// #include <future>

/*
    TODO LIST:
    - feat: async chunk loading? i used std::async (see comments in chunk
   loading code) but im not sure if it makes a difference
    i would need to conduct more tests :\
    - feat: chunk updates
    - feat: chunk unloading?
    - fix: initial chunk that gets rendered off the rip has weird alpha blending
    artifacts when it
*/

class TPoint3D {
  public:
    TPoint3D(float x, float y, float z) : x(x), y(y), z(z){};

    float x, y, z;
};

struct hashFunc {
    size_t operator()(const TPoint3D &k) const {
        size_t h1 = std::hash<float>()(k.x);
        size_t h2 = std::hash<float>()(k.y);
        size_t h3 = std::hash<float>()(k.z);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

struct equalsFunc {
    bool operator()(const TPoint3D &lhs, const TPoint3D &rhs) const {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }
};

typedef std::vector<Chunk *> ChunkList;
typedef std::unordered_map<TPoint3D, Chunk *, hashFunc, equalsFunc> ChunkMap;

struct ChunkManager {
    static int const ASYNC_NUM_CHUNKS_PER_FRAME = 5;
    ChunkManager(unsigned int chunkAddDistance);
    ~ChunkManager();
    void Update(float dt, Vector3 newCameraPosition, Vector3 newCameraLookAt);
    void UpdateAsyncChunker(Vector3 newCameraPosition);
    void UpdateLoadList();
    void UpdateSetupList();
    void UpdateRebuildList();
    void UpdateFlagsList();
    void UpdateUnloadList();
    void UpdateRenderList();
    void UpdateVisibilityList(Vector3 newCameraPosition);
    void Render();

    ChunkMap chunks;
    ChunkList chunkLoadList;
    ChunkList chunkRenderList;
    ChunkList chunkUnloadList;
    ChunkList chunkVisibilityList;
    ChunkList chunkSetupList;

    bool genChunk;
    bool forceVisibilityUpdate;
    Vector3 cameraPosition;
    Vector3 cameraLookAt;
    unsigned int chunkAddDistance;
};

ChunkManager::ChunkManager(unsigned int _chunkAddDistance) {
    chunkAddDistance = _chunkAddDistance;
    genChunk = true;
    bool forceVisibilityUpdate = true;
}

ChunkManager::~ChunkManager() { chunks.clear(); }

void ChunkManager::Update(float dt, Vector3 newCameraPosition,
                          Vector3 newCameraLookAt) {

    if (genChunk) {
        UpdateAsyncChunker(newCameraPosition);
        // std::async(std::launch::async, &ChunkManager::UpdateAsyncChunker,
        // this, newCameraPosition);
    }
    UpdateLoadList();
    // std::async(std::launch::async, &ChunkManager::UpdateLoadList, this);
    UpdateSetupList();
    // std::async(std::launch::async, &ChunkManager::UpdateSetupList, this);
    // UpdateRebuildList();
    // UpdateFlagsList();
    // UpdateUnloadList();
    UpdateVisibilityList(newCameraPosition);
    UpdateRenderList();
    cameraPosition = newCameraPosition;
    cameraLookAt = newCameraLookAt;
}

float roundUp(float number, float fixedBase) {
    if (fixedBase != 0 && number != 0) {
        float sign = number > 0 ? 1 : -1;
        number *= sign;
        number /= fixedBase;
        int fixedPoint = (int)ceil(number);
        number = fixedPoint * fixedBase;
        number *= sign;
    }
    return number;
}

void ChunkManager::UpdateAsyncChunker(Vector3 newCameraPosition) {
    if (Vector3Equals(newCameraPosition, cameraPosition)) {
        return;
    }

    // generate chunks inside render distance cube
    ChunkList::iterator iterator;
    float startX = roundUp(newCameraPosition.x -
                               ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                                Block::BLOCK_RENDER_SIZE),
                           Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    float endX = roundUp(newCameraPosition.x +
                             ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                              Block::BLOCK_RENDER_SIZE),
                         Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    float startY = roundUp(newCameraPosition.y -
                               ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                                Block::BLOCK_RENDER_SIZE),
                           Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    float endY = roundUp(newCameraPosition.y +
                             ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                              Block::BLOCK_RENDER_SIZE),
                         Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    float startZ = roundUp(newCameraPosition.z -
                               ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                                Block::BLOCK_RENDER_SIZE),
                           Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    float endZ = roundUp(newCameraPosition.z +
                             ((float)chunkAddDistance * Chunk::CHUNK_SIZE *
                              Block::BLOCK_RENDER_SIZE),
                         Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);

    // printf("start: (%06.3f, %06.3f), end: (%06.3f, %06.3f)", startX, startY,
    //    endX, endY);
    for (float i = startX; i < endX;
         i += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
        for (float j = startY; j < endY;
             j += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
            for (float k = startZ; k < endZ;
                 k += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
                // generate flat for now
                if (j > -Block::BLOCK_RENDER_SIZE) {
                    continue;
                }
                TPoint3D coords = {i, j, k};
                if (chunks.find(coords) != chunks.end()) {
                    Chunk *currChunk = chunks.at(coords);
                    if (!currChunk->isLoaded()) {
                        chunkVisibilityList.push_back(currChunk);
                    }
                    continue;
                }
                // create new chunk
                Chunk *newChunk = new Chunk({coords.x, coords.y, coords.z});
                chunks[coords] = newChunk;
                chunkVisibilityList.push_back(newChunk);
            }
        }
    }
}

void ChunkManager::UpdateLoadList() {
    int lNumOfChunksLoaded = 0;
    ChunkList::iterator iterator;
    for (iterator = chunkLoadList.begin();
         iterator != chunkLoadList.end() &&
         (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME);
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() == false) {
            if (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME) {
                pChunk->load();
                lNumOfChunksLoaded++;
                forceVisibilityUpdate = true;
            }
        }
    } // Clear the load list (every frame)
    chunkLoadList.clear();
}

void ChunkManager::UpdateSetupList() { // Setup any chunks that have not
                                       // already been setup
    ChunkList::iterator iterator;
    for (iterator = chunkSetupList.begin(); iterator != chunkSetupList.end();
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() && pChunk->isSetup() == false) {
            pChunk->setup();
            if (pChunk->isSetup()) { // Only force the visibility update if we
                                     // actually setup the chunk, some chunks
                                     // wait in the pre-setup stage...
                forceVisibilityUpdate = true;
            }
        }
    } // Clear the setup list (every frame)
    chunkSetupList.clear();
}

void ChunkManager::UpdateRenderList() {
    // Clear the render list each frame BEFORE we do our tests to see what
    // chunks should be rendered
    chunkRenderList.clear();
    ChunkList::iterator iterator;
    for (iterator = chunkVisibilityList.begin();
         iterator != chunkVisibilityList.end(); ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk != NULL) {
            if (pChunk->isLoaded() && pChunk->isSetup()) {
                chunkRenderList.push_back(pChunk);
            }
        }
    }
}

void ChunkManager::UpdateVisibilityList(Vector3 newCameraPosition) {
    for (Chunk *chunk : chunkVisibilityList) {
        chunkLoadList.push_back(chunk);
        chunkSetupList.push_back(chunk);
        chunkRenderList.push_back(chunk);
    }
}

void ChunkManager::Render() {
    for (Chunk *chunk : chunkRenderList) {
        chunk->render();
    }
}

#endif // CHUNK_MANAGER