#ifndef BLOCK_H
#define BLOCK_H

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
    static constexpr float BLOCK_RENDER_SIZE = 2.0f;
    // TODO: do we keep this in CPU or in GPU ?
    bool isActive;    Block(){};
    ~Block(){};
    BlockType blockType;
};

#endif // BLOCK_H