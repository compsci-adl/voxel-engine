#ifndef BLOCK_H
#define BLOCK_H

enum BlockType {
    Default,
    Grass,
    Sand,
    Dirt,
    Water,
    Stone,
    Wood,
    NumTypes,
};

struct Block {
    static constexpr int BLOCK_RENDER_SIZE = 2;
    // TODO: do we keep this in CPU or in GPU ?
    bool isActive;    Block(){};
    ~Block(){};
    BlockType blockType;
};

#endif // BLOCK_H