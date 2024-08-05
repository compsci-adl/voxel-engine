#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include "Chunk.h"
#include "raylib.h"
#include <raymath.h>
#include <unordered_map>
#include <vector>

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

struct equalsFunc{
  bool operator()( const TPoint3D& lhs, const TPoint3D& rhs ) const{
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
}

ChunkManager::~ChunkManager() {
    chunks.clear();
}

void ChunkManager::Update(float dt, Vector3 newCameraPosition,
                          Vector3 newCameraLookAt) {

    if(genChunk) {
        UpdateAsyncChunker(newCameraPosition);
    }
    UpdateLoadList();
    UpdateSetupList();
    // UpdateRebuildList();
    // UpdateFlagsList();
    // UpdateUnloadList();
    UpdateVisibilityList(newCameraPosition);
    if (!Vector3Equals(cameraPosition, newCameraPosition) ||
        !Vector3Equals(cameraLookAt, newCameraLookAt)) {
        UpdateRenderList();
    }
    cameraPosition = newCameraPosition;
    cameraLookAt = newCameraLookAt;
}

int roundUp(int numToRound, int multiple) {
    if (multiple < 0)
        return 0;
    int isPositive = (int)(numToRound >= 0);
    return ((numToRound + isPositive * (multiple - 1)) / multiple) * multiple;
}

void ChunkManager::UpdateAsyncChunker(Vector3 newCameraPosition) {
    if (Vector3Equals(newCameraPosition, cameraPosition)) {
        return;
    }
    // generate chunks inside render distance rectangle
    // Rectangle renderRect = {newCameraPosition.x - chunkAddDistance,
    //                         newCameraPosition.z + chunkAddDistance,
    //                         chunkAddDistance, chunkAddDistance};
    ChunkList::iterator iterator;
    int startX =
        roundUp(newCameraPosition.x - (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);
    int endX =
        roundUp(newCameraPosition.x + (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);
    int startY =
        roundUp(newCameraPosition.y - (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);
    int endY =
        roundUp(newCameraPosition.y + (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);
    int startZ =
        roundUp(newCameraPosition.z - (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);
    int endZ =
        roundUp(newCameraPosition.z + (chunkAddDistance * Chunk::CHUNK_SIZE),
                Chunk::CHUNK_SIZE);

    for (float i = startX; i < endX; i += Chunk::CHUNK_SIZE) {
        for (float j = startY; j < endY; j += Chunk::CHUNK_SIZE) {
            for (float k = startZ; k < endZ; k += Chunk::CHUNK_SIZE) {
                // generate flat for now
                if (j > 1) {
                    continue;
                }
                TPoint3D coords = {Block::BLOCK_RENDER_SIZE * (float)i, Block::BLOCK_RENDER_SIZE * (float)j, Block::BLOCK_RENDER_SIZE * (float)k};
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
    for (iterator = chunkVisibilityList.begin(); iterator != chunkVisibilityList.end();
         ++iterator) {
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

// void ChunkManager::UpdateRenderList() {
//   // Clear the render list each frame BEFORE we do our tests to see what
//   chunks should be rendered chunkRenderList.clear(); ChunkList::iterator
//   iterator; for (iterator = chunkRenderList.begin(); iterator !=
//   chunkRenderList.end(); ++iterator) {
//     Chunk * pChunk = ( * iterator);
//     if (pChunk != NULL) {
//       if (pChunk -> isLoaded() && pChunk -> isSetup()) {
//         if (pChunk -> ShouldRender()) // Early flags check so we don't
//         always have to do the frustum check...
//         {// Check if this chunk is inside the camera frustum
//           float c_offset = (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)
//           - Block::BLOCK_RENDER_SIZE; Vector3d chunkCenter = pChunk ->
//           GetPosition() + Vector3d(c_offset, c_offset, c_offset); float
//           c_size = Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE; if
//           (m_pRenderer -> CubeInFrustum(m_pRenderer ->
//           GetActiveViewPort(), chunkCenter, c_size, c_size, c_size)) {
//             m_vpChunkRenderList.push_back(pChunk);
//           }
//         }
//       }
//     }
//   }
// }

#endif // CHUNK_MANAGER