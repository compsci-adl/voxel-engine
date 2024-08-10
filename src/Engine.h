#ifndef ENGINE_H
#define ENGINE_H

#include "ChunkManager.h";

struct Engine {
    ChunkManager chunkManager;
    // This will have to do for now
    Engine(unsigned int chunkAddDistance);
    ~Engine();
    void update();
    void render();
};

// Engine::Engine(unsigned int chunkAddDistance){
//     chunkManager = ChunkManager(chunkAddDistance);
// }

// void Engine::update() {
//     chunkManager.update();
// }

// void Engine::render() {
//     chunkManager.render();
// }



#endif // ENGINE_H