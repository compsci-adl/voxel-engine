#ifndef BLOCK_H
#define BLOCK_H
#include <raylib.h>

enum BlockType {
    BlockType_Default = 0,
    BlockType_Grass,
    BlockType_Dirt,
    BlockType_Water,
    BlockType_Stone,
    BlockType_Wood,
    BlockType_Sand,
    BlockType_NumTypes,
};

struct Block {
    static constexpr float BLOCK_RENDER_SIZE = 0.1f;
    bool isActive;
    Block(){};
    ~Block(){};
    BlockType blockType;
};

#endif // BLOCK_H