#ifndef BLOCK_H
#define BLOCK_H
#include <raylib.h>

enum BlockType {
    Default,
    Grass,
    Dirt,
    Water,
    Stone,
    Wood,
    Sand,
    NumTypes,
};

struct Block {
    static constexpr float BLOCK_RENDER_SIZE = 1.0f;
    bool isActive;
    Block(){};
    ~Block(){};
    BlockType blockType;
};

#endif // BLOCK_H