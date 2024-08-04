#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include <vector>

typedef std::vector<Chunk*> ChunkList;

struct ChunkManager {
    static int ASYNC_NUM_CHUNKS_PER_FRAME = 5;
    ChunkManager();
    ~ChunkManager();
    void Update();
    void UpdateAsyncChunker();
    void UpdateLoadList();
    void UpdateSetupList();
    void UpdateRebuildList();
    void UpdateFlagsList();
    void UpdateUnloadList();

    ChunkList chunkLoadList;
    ChunkList chunkRenderList;
    ChunkList chunkUnloadList;
    ChunkList chunkVisibilityList;
    ChunkList chunkSetupList;

    bool forceVisibilityUpdate;
    Vector3d cameraPosition;
    Vector3d cameraLookAt;
}

void ChunkManager::Update(float dt, Vector3d newCameraPosition, Vector3d newCameraLookAt) {
    UpdateAsyncChunker();
    UpdateLoadList();
    UpdateSetupList();
    UpdateRebuildList();
    UpdateFlagsList();
    UpdateUnloadList();
    UpdateVisibilityList(newCameraPosition);
    if (cameraPosition != newCameraPosition || cameraLookAt != newCameraLookAt) {
        UpdateRenderList();
    }
    cameraPosition = newCameraPosition;
    cameraLookAt = newCameraLookAt;
}

void ChunkManager::UpdateLoadList() {
    int lNumOfChunksLoaded = 0;
    ChunkList::iterator iterator;
    for (iterator = chunkLoadList.begin();
         iterator != chunkLoadList.end() &&
         (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME);
         ++iterator) {
        Chunk* pChunk = (*iterator);
        if (pChunk->IsLoaded == false) {
            if (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME)) {
                    pChunk->isLoaded = true; // Increase the chunks loaded count
                    lNumOfChunksLoaded++;
                    forceVisibilityUpdate = true;
                }
        }
    } // Clear the load list (every frame)
    chunkLoadList.clear();
}

#endif // CHUNK_MANAGER