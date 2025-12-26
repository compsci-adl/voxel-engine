#ifndef BLOCK_H
#define BLOCK_H

#include <unordered_map>
#include <vector>
#include <utility> // for std::pair

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

// create map of texture coordinates for each size and type
// stores coordinates as x,y int, so top left 0, 0. to the right is 1, 0, below is 0, 1 and bottom right is 15, 15
// input top left corner, will get 0,0 to 1,1
// NOTE: Maximum textures in map is 63x63 icons due to 5 bit coordinates.
// front, back, left, right, top, bottom
std::unordered_map<int, std::vector<std::pair<int, int>>> textureCoordMap = {
    {BlockType::Grass, {{3, 0}, {3, 0}, {3, 0}, {3, 0}, {0, 0}, {2, 0}}},
    {BlockType::Sand, {{0, 11}, {0, 11}, {0, 11}, {0, 11}, {0, 11}, {0, 11}}},
    {BlockType::Dirt, {{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}}},
    {BlockType::Water, {{13, 12}, {13, 12}, {13, 12}, {13, 12}, {13, 12}, {13, 12}}},
    {BlockType::Stone, {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},
    {BlockType::Wood, {{4, 0}, {4, 0}, {4, 0}, {4, 0}, {4, 0}, {4, 0}}}
};



class Block {
    static void addColCubeFace(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 col, float brightness,
                        std::vector<float> &vertices, std::vector<unsigned int> &indices, int &vCount, int &iCount);
public:
    static constexpr int BLOCK_RENDER_SIZE = 2;
    // TODO: do we keep this in CPU or in GPU ?
    bool isActive = false;    
    Block(){};
    ~Block(){};
    BlockType blockType =  BlockType::Default;

    

    static void creatColourCube(glm::vec3 pos, glm::vec3 sie, glm::vec3 colour,
                                std::vector<float> &vertices, std::vector<unsigned int> &indices, int &vCount, int &iCount);
    
   // static void createTexCube(glm::vec3 pos, glm::vec3 size,
    //                            std::vector<float> &vertices, std::vector<unsigned int> &indices, int *vCount, int *iCount,
     //                           BlockType blockType);


};



void Block::addColCubeFace(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 col, float brightness,
                         std::vector<float> &vertices, std::vector<unsigned int> &indices, int &vCount, int &iCount) {
    // 7 floats per vertex: x, y, z, r, g, b, brightness
    glm::vec3 points[4] = {p1, p2, p3, p4};
    int index_offset = vCount / 7;

    // Add vertices using push_back
    for (int i = 0; i < 4; i++) {
        vertices.push_back(points[i].x);
        vertices.push_back(points[i].y);
        vertices.push_back(points[i].z);
        vertices.push_back(col.x);
        vertices.push_back(col.y);
        vertices.push_back(col.z);
        vertices.push_back(brightness);
    }

    // Add indices using push_back
    indices.push_back(index_offset);
    indices.push_back(index_offset + 1);
    indices.push_back(index_offset + 2);
    indices.push_back(index_offset);
    indices.push_back(index_offset + 2);
    indices.push_back(index_offset + 3);

    vCount += 28; // 7 floats per vertex, 4 vertices per face
    iCount += 6;
}


// no neighbouring checking
// no compression 
void Block::creatColourCube(glm::vec3 pos, glm::vec3 size, glm::vec3 colour,
                    std::vector<float> &vertices, std::vector<unsigned int> &indices, int &vCount, int &iCount){
    float blockX = pos.x;
    float blockY = pos.y;
    float blockZ = pos.z;

    glm::vec3 p1 = {Block::BLOCK_RENDER_SIZE * blockX - size.x,
                                Block::BLOCK_RENDER_SIZE * blockY - size.y,
                                Block::BLOCK_RENDER_SIZE * blockZ + size.z};

    glm::vec3 p2 = {Block::BLOCK_RENDER_SIZE * blockX + size.x,
                               Block::BLOCK_RENDER_SIZE * blockY - size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ + size.z};

    glm::vec3 p3 = {Block::BLOCK_RENDER_SIZE * blockX + size.x,
                               Block::BLOCK_RENDER_SIZE * blockY + size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ + size.z};

    glm::vec3 p4 = {Block::BLOCK_RENDER_SIZE * blockX - size.x,
                               Block::BLOCK_RENDER_SIZE * blockY + size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ + size.z};

    glm::vec3 p5 = {Block::BLOCK_RENDER_SIZE * blockX + size.x,
                               Block::BLOCK_RENDER_SIZE * blockY - size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ - size.z};

    glm::vec3 p6 = {Block::BLOCK_RENDER_SIZE * blockX - size.x,
                               Block::BLOCK_RENDER_SIZE * blockY - size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ - size.z};

    glm::vec3 p7 = {Block::BLOCK_RENDER_SIZE * blockX - size.x,
                               Block::BLOCK_RENDER_SIZE * blockY + size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ - size.z};

    glm::vec3 p8 = {Block::BLOCK_RENDER_SIZE * blockX + size.x,
                               Block::BLOCK_RENDER_SIZE * blockY + size.y,
                               Block::BLOCK_RENDER_SIZE * blockZ - size.z};

    // Reserve space for vertices and indices to avoid repeated reallocations
    vertices.reserve(vertices.size() + 28 * 6); // 6 faces, 28 floats per face
    indices.reserve(indices.size() + 6 * 6);    // 6 faces, 6 indices per face

    // front, back, left, right, top, bottom
    addColCubeFace(p1, p2, p3, p4, colour, 0.86f, vertices, indices, vCount, iCount); // front
    addColCubeFace(p5, p6, p7, p8, colour, 0.86f, vertices, indices, vCount, iCount); // back
    addColCubeFace(p2, p5, p8, p3, colour, 0.8f, vertices, indices, vCount, iCount);  // left
    addColCubeFace(p6, p1, p4, p7, colour, 0.8f, vertices, indices, vCount, iCount);  // right
    addColCubeFace(p4, p3, p8, p7, colour, 1.0f, vertices, indices, vCount, iCount);  // top
    addColCubeFace(p6, p5, p2, p1, colour, 0.7f, vertices, indices, vCount, iCount);  // bottom
}



#endif // BLOCK_H