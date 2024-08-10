#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include "Chunk.h"
#include "Frustum.h"
#include "raylib.h"
#include <raymath.h>
// #include <future>
#include <unordered_map>
#include <vector>

/*
    TODO LIST:
    - feat: async chunk loading?
    - feat: chunk unloading?
    - fix: some chunks (def. first one) has weird alpha rendering bug - need to
   investigate the cause of this later.
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
    static int const ASYNC_NUM_CHUNKS_PER_FRAME = 12;
    ChunkManager(unsigned int chunkAddDistance);
    ~ChunkManager();
    void update(float dt, Vector3 newCameraPosition, Vector3 newCameraLookAt);
    void updateAsyncChunker(Vector3 newCameraPosition);
    void updateLoadList();
    void updateSetupList();
    void updateRebuildList();
    void updateFlagsList();
    void updateUnloadList(Vector3 newCameraPosition);
    void updateVisibilityList(Vector3 newCameraPosition);
    void updateRenderList();

    void QueueChunkToRebuild(Chunk *chunk);
    std::pair<Vector3, Vector3> GetChunkRange(Vector3 newCameraPosition);
    void render();

    ChunkMap chunks;
    ChunkList chunkLoadList;
    ChunkList chunkSetupList;
    ChunkList chunkRebuildList;
    ChunkList chunkRenderList;
    ChunkList chunkUnloadList;
    ChunkList chunkVisibilityList;
    Frustum frustum;

    bool genChunk;
    bool forceVisibilityupdate;
    Vector3 cameraPosition;
    Vector3 cameraLookAt;
    unsigned int chunkAddDistance;
};

ChunkManager::ChunkManager(unsigned int _chunkAddDistance) {
    chunkAddDistance = _chunkAddDistance;
    genChunk = true;
    bool forceVisibilityupdate = true;
}

ChunkManager::~ChunkManager() { chunks.clear(); }

void ChunkManager::update(float dt, Vector3 newCameraPosition,
                          Vector3 newCameraLookAt) {
    if (genChunk) {
        updateAsyncChunker(newCameraPosition);
        // asyncChunkFuture = std::async(&ChunkManager::updateAsyncChunker,
        // this,
        //    newCameraPosition);
    }
    updateLoadList();
    // std::async(std::launch::async, &ChunkManager::updateLoadList, this);
    updateSetupList();
    // std::async(std::launch::async, &ChunkManager::updateSetupList, this);
    updateRebuildList();
    // updateFlagsList();
    // updateUnloadList(newCameraPosition);
    updateVisibilityList(newCameraPosition);
    updateRenderList();
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

std::pair<Vector3, Vector3>
ChunkManager::GetChunkRange(Vector3 newCameraPosition) {
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

    return std::pair<Vector3, Vector3>({startX, startY, startZ},
                                       {endX, endY, endZ});
}

void ChunkManager::updateAsyncChunker(Vector3 newCameraPosition) {
    if (Vector3Equals(newCameraPosition, cameraPosition)) {
        return;
    }

    std::pair<Vector3, Vector3> chunkRange = GetChunkRange(newCameraPosition);
    Vector3 start = chunkRange.first;
    Vector3 end = chunkRange.second;

    // generate chunks inside render distance cube
    ChunkList::iterator iterator;

    // printf("start: (%06.3f, %06.3f), end: (%06.3f, %06.3f)", startX, startY,
    //    endX, endY);
    for (float i = start.x; i < end.x;
         i += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
        for (float j = start.y; j < end.y;
             j += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
            for (float k = start.z; k < end.z;
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

void ChunkManager::updateLoadList() {
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
                forceVisibilityupdate = true;
            }
        }
    } // Clear the load list (every frame)
    chunkLoadList.clear();
}

void ChunkManager::updateSetupList() { // Setup any chunks that have not
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
                forceVisibilityupdate = true;
            }
        }
    } // Clear the setup list (every frame)
    chunkSetupList.clear();
}

void ChunkManager::QueueChunkToRebuild(Chunk *chunk) {
    chunkRebuildList.push_back(chunk);
}

void ChunkManager::updateRebuildList() {
    // Rebuild any chunks that are in the rebuild chunk list
    ChunkList::iterator iterator;
    int lNumRebuiltChunkThisFrame = 0;
    for (iterator = chunkRebuildList.begin();
         iterator != chunkRebuildList.end() &&
         (lNumRebuiltChunkThisFrame != ASYNC_NUM_CHUNKS_PER_FRAME);
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() && pChunk->isSetup()) {
            if (lNumRebuiltChunkThisFrame != ASYNC_NUM_CHUNKS_PER_FRAME) {
                pChunk->rebuildMesh(); // If we rebuild a chunk, add it to the
                                       // list of chunks that need their render
                                       // flags updated
                // since we might now be empty or surrounded
                // m_vpChunkupdateFlagsList.push_back(pChunk); // Also add our
                // neighbours since they might now be surrounded too (If we have
                // neighbours) Chunk * pChunkXMinus = GetChunk(pChunk -> GetX()
                // - 1, pChunk -> GetY(), pChunk -> GetZ()); Chunk * pChunkXPlus
                // = GetChunk(pChunk -> GetX() + 1, pChunk -> GetY(), pChunk ->
                // GetZ()); Chunk * pChunkYMinus = GetChunk(pChunk -> GetX(),
                // pChunk -> GetY() - 1, pChunk -> GetZ()); Chunk * pChunkYPlus
                // = GetChunk(pChunk -> GetX(), pChunk -> GetY() + 1, pChunk ->
                // GetZ()); Chunk * pChunkZMinus = GetChunk(pChunk -> GetX(),
                // pChunk -> GetY(), pChunk -> GetZ() - 1); Chunk * pChunkZPlus
                // = GetChunk(pChunk -> GetX(), pChunk -> GetY(), pChunk ->
                // GetZ() + 1); if (pChunkXMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkXMinus); if
                // (pChunkXPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkXPlus); if
                // (pChunkYMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkYMinus); if
                // (pChunkYPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkYPlus); if
                // (pChunkZMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkZMinus); if
                // (pChunkZPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkZPlus); // Only
                // rebuild a certain number of chunks per frame
                lNumRebuiltChunkThisFrame++;
                forceVisibilityupdate = true;
            }
        }
    }
    // Clear the rebuild list
    chunkRebuildList.clear();
}

// unload chunks
// void ChunkManager::updateUnloadList(Vector3 newCameraPosition) {
//     ChunkList::iterator iterator;
//     for (iterator = chunkUnloadList.begin(); iterator !=
//     chunkUnloadList.end(); iterator++) {
//         Chunk *pChunk = (*iterator);
//         if (pChunk->isLoaded()) {
//             // TODO: async here?
//             std::pair<Vector3, Vector3> chunkRange =
//             GetChunkRange(newCameraPosition); Vector3 start =
//             chunkRange.first; Vector3 end = chunkRange.second; if(!(start.x
//             <= pChunk->chunkPosition.x <= end.x and start.y <=
//             pChunk->chunkPosition.y <= end.y  and start.z <=
//             pChunk->chunkPosition.z <= end.z)) {
//                 chunks.erase(TPoint3D(pChunk->chunkPosition.x,
//                 pChunk->chunkPosition.y, pChunk->chunkPosition.z));
//                 pChunk->unload();
//                 // delete pChunk;
//             }
//         }
//     }
//     chunkUnloadList.clear();
// }

void ChunkManager::updateRenderList() {
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

void ChunkManager::updateVisibilityList(Vector3 newCameraPosition) {
    for (Chunk *chunk : chunkVisibilityList) {
        chunkLoadList.push_back(chunk);
        // chunkUnloadList.push_back(chunk);
        chunkSetupList.push_back(chunk);
        chunkRenderList.push_back(chunk);
    }
}

void ChunkManager::render() {
    for (Chunk *chunk : chunkRenderList) {
        chunk->render();
    }
}

#endif // CHUNK_MANAGER