#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include "Chunk.h"
#include <learnopengl/shader_m.h>
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
    ChunkManager();
    ChunkManager(unsigned int _chunkAddDistance, Shader *_terrainShader);
    ~ChunkManager();
    void update(float dt, glm::vec3 newCameraPosition,
                glm::vec3 newCameraLookAt);
    void updateAsyncChunker(glm::vec3 newCameraPosition);
    void updateLoadList();
    void updateSetupList();
    void updateRebuildList();
    void updateFlagsList();
    void updateUnloadList(glm::vec3 newCameraPosition);
    void updateVisibilityList(glm::vec3 newCameraPosition);
    void updateRenderList(glm::vec3 newCameraPosition);

    void QueueChunkToRebuild(Chunk *chunk);
    std::pair<glm::vec3, glm::vec3> GetChunkRange(glm::vec3 newCameraPosition);
    void render();

    Shader *terrainShader;

    ChunkMap chunks;
    ChunkList chunkLoadList;
    ChunkList chunkSetupList;
    ChunkList chunkRebuildList;
    ChunkList chunkRenderList;
    ChunkList chunkUnloadList;
    ChunkList chunkVisibilityList;
    // Frustum frustum;

    bool genChunk;
    bool forceVisibilityupdate;
    glm::vec3 cameraPosition;
    glm::vec3 cameraLookAt;
    unsigned int chunkAddDistance;
};
ChunkManager::ChunkManager() {}

ChunkManager::ChunkManager(unsigned int _chunkAddDistance,
                           Shader *_terrainShader) {
    chunkAddDistance = _chunkAddDistance;
    terrainShader = _terrainShader;
    genChunk = true;
    bool forceVisibilityupdate = true;
}

ChunkManager::~ChunkManager() { chunks.clear(); }

void ChunkManager::update(float dt, glm::vec3 newCameraPosition,
                          glm::vec3 newCameraLookAt) {
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
    updateUnloadList(newCameraPosition);
    updateVisibilityList(newCameraPosition);
    updateRenderList(newCameraPosition);
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

std::pair<glm::vec3, glm::vec3>
ChunkManager::GetChunkRange(glm::vec3 newCameraPosition) {
    int startX = (int)roundUp(
        newCameraPosition.x -
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endX = (int)roundUp(
        newCameraPosition.x +
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startY = (int)roundUp(
        newCameraPosition.y -
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endY = (int)roundUp(
        newCameraPosition.y +
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startZ = (int)roundUp(
        newCameraPosition.z -
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endZ = (int)roundUp(
        newCameraPosition.z +
            (chunkAddDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);

    return std::pair<glm::vec3, glm::vec3>({startX, startY, startZ},
                                           {endX, endY, endZ});
}

void ChunkManager::updateAsyncChunker(glm::vec3 newCameraPosition) {
    if (newCameraPosition == cameraPosition) {
        return;
    }

    std::pair<glm::vec3, glm::vec3> chunkRange =
        GetChunkRange(newCameraPosition);
    glm::vec3 start = chunkRange.first;
    glm::vec3 end = chunkRange.second;

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
                Chunk *newChunk =
                    new Chunk({coords.x, coords.y, coords.z}, terrainShader);
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
void ChunkManager::updateUnloadList(glm::vec3 newCameraPosition) {
    ChunkList::iterator iterator;
    for (iterator = chunkUnloadList.begin(); iterator != chunkUnloadList.end();
         iterator++) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded()) {
            // TODO: async here?
            std::pair<glm::vec3, glm::vec3> chunkRange =
                GetChunkRange(newCameraPosition);
            glm::vec3 start = chunkRange.first;
            glm::vec3 end = chunkRange.second;
            if (!((start.x <= pChunk->chunkPosition.x &&
                   pChunk->chunkPosition.x <= end.x) &&
                  (start.y <= pChunk->chunkPosition.y &&
                   pChunk->chunkPosition.y <= end.y) &&
                  (start.z <= pChunk->chunkPosition.z &&
                   pChunk->chunkPosition.z <= end.z))) {
                pChunk->unload();
                chunks.erase(TPoint3D(pChunk->chunkPosition.x,
                                      pChunk->chunkPosition.y,
                                      pChunk->chunkPosition.z));
                // delete pChunk;
            }
        }
    }
    chunkUnloadList.clear();
}

void ChunkManager::updateRenderList(glm::vec3 newCameraPosition) {
    // Clear the render list each frame BEFORE we do our tests to see what
    // chunks should be rendered
    chunkRenderList.clear();
    ChunkList::iterator iterator;
    for (iterator = chunkVisibilityList.begin();
         iterator != chunkVisibilityList.end(); ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk != NULL) {
            if (pChunk->isLoaded() && pChunk->isSetup()) {

                std::pair<glm::vec3, glm::vec3> chunkRange =
                    GetChunkRange(newCameraPosition);
                glm::vec3 start = chunkRange.first;
                glm::vec3 end = chunkRange.second;
                if (((start.x <= pChunk->chunkPosition.x &&
                      pChunk->chunkPosition.x <= end.x) &&
                     (start.y <= pChunk->chunkPosition.y &&
                      pChunk->chunkPosition.y <= end.y) &&
                     (start.z <= pChunk->chunkPosition.z &&
                      pChunk->chunkPosition.z <= end.z))) {
                    chunkRenderList.push_back(pChunk);
                    // delete pChunk;
                }
                // chunkRenderList.push_back(pChunk);
            }
        }
    }
}

void ChunkManager::updateVisibilityList(glm::vec3 newCameraPosition) {
    for (Chunk *chunk : chunkVisibilityList) {
        chunkLoadList.push_back(chunk);
        // chunkUnloadList.push_back(chunk);
        chunkSetupList.push_back(chunk);
    }
}

void ChunkManager::render() {
    for (Chunk *chunk : chunkRenderList) {
        chunk->render();
    }
}

#endif // CHUNK_MANAGER