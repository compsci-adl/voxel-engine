#ifndef CHUNK_H
#define CHUNK_H
#include "Block.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>

/*
    TODO LIST:
    - chunk render func inside our outside? how do we want to style our
   codebase?
*/

struct Chunk {
    static const int CHUNK_SIZE = 16;
    Block ***blocks;
    Mesh mesh;
    Model model;
    bool isLoaded;

    Chunk() {
        blocks = new Block **[CHUNK_SIZE];
        for (int i = 0; i < CHUNK_SIZE; i++) {
            blocks[i] = new Block *[CHUNK_SIZE];
            for (int j = 0; j < CHUNK_SIZE; j++) {
                blocks[i][j] = new Block[CHUNK_SIZE];
            }
        }
    };

    ~Chunk() {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                delete[] blocks[i][j];
            }
            delete[] blocks[i];
        }
        delete[] blocks;
    };

    // create vbo to be used to render chunk
    void createMesh() {
        int vertexCount = 0;
        int indexCount = 0;

        int totalVertices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 4;
        int totalIndices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6;

        float *normals = (float *)malloc(totalVertices * 3 * sizeof(float));
        float *texcoords = (float *)malloc(totalVertices * 2 * sizeof(float));
        unsigned short *indices =
            (unsigned short *)malloc(totalIndices * sizeof(unsigned short));
        unsigned char *colors =
            (unsigned char *)malloc(totalVertices * 4 * sizeof(unsigned char));

        mesh = {0};
        mesh.vertexCount = 0;
        mesh.triangleCount = 0;
        mesh.vertices = (float *)malloc(totalVertices * 3 * sizeof(float));
        mesh.normals = normals;
        mesh.texcoords = texcoords;
        mesh.indices = indices;
        mesh.colors = colors;

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    // TODO: im storing the pointers to the meshes in RAM - is
                    // this how it should be done?
                    if (blocks[x][y][z].isActive) {
                        Vector3 pos = {Block::BLOCK_RENDER_SIZE * (float)x,
                                       Block::BLOCK_RENDER_SIZE * (float)y,
                                       Block::BLOCK_RENDER_SIZE * (float)z};
                        CreateCube(&mesh, pos, Block::BLOCK_RENDER_SIZE,
                                   &mesh.vertexCount, &indexCount);
                    }
                }
            }
        }

        mesh.triangleCount = indexCount / 3;
    }

    // renders the chunk
    void render() {
        Color color = DARKGREEN;
        color.a = 100.0f;
        DrawModel(model, {0.0, 0.0, 0.0}, 1.0f, color);
        DrawModelWires(model, {0.0, 0.0, 0.0}, 1.0f, WHITE);
    }

    void update() {
        UploadMesh(&mesh, false);
        model = LoadModelFromMesh(mesh);
    }

    void randomize() {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int i = rand() % 2;
                    // blocks[x][y][z].isActive = i == 0 ? false : true;
                    blocks[x][y][z].isActive = true;
                }
            }
        }
    }

    void AddCubeFace(Mesh *mesh, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4,
                     Vector3 normal, int *vCount, int *iCount, float r, float g,
                     float b, float a) {
        int v1 = *vCount;
        int v2 = *vCount + 1;
        int v3 = *vCount + 2;
        int v4 = *vCount + 3;

        // Add vertices
        mesh->vertices[v1 * 3] = p1.x;
        mesh->vertices[v1 * 3 + 1] = p1.y;
        mesh->vertices[v1 * 3 + 2] = p1.z;
        mesh->vertices[v2 * 3] = p2.x;
        mesh->vertices[v2 * 3 + 1] = p2.y;
        mesh->vertices[v2 * 3 + 2] = p2.z;
        mesh->vertices[v3 * 3] = p3.x;
        mesh->vertices[v3 * 3 + 1] = p3.y;
        mesh->vertices[v3 * 3 + 2] = p3.z;
        mesh->vertices[v4 * 3] = p4.x;
        mesh->vertices[v4 * 3 + 1] = p4.y;
        mesh->vertices[v4 * 3 + 2] = p4.z;

        // Add normals
        for (int i = 0; i < 4; i++) {
            mesh->normals[(*vCount + i) * 3] = normal.x;
            mesh->normals[(*vCount + i) * 3 + 1] = normal.y;
            mesh->normals[(*vCount + i) * 3 + 2] = normal.z;
        }

        // Add colors
        for (int i = 0; i < 4; i++) {
            mesh->colors[(*vCount + i) * 4] = r;
            mesh->colors[(*vCount + i) * 4 + 1] = g;
            mesh->colors[(*vCount + i) * 4 + 2] = b;
            mesh->colors[(*vCount + i) * 4 + 3] = a;
        }

        // Add texture coordinates (dummy values)
        mesh->texcoords[(*vCount) * 2] = 0.0f;
        mesh->texcoords[(*vCount) * 2 + 1] = 0.0f;
        mesh->texcoords[(*vCount + 1) * 2] = 1.0f;
        mesh->texcoords[(*vCount + 1) * 2 + 1] = 0.0f;
        mesh->texcoords[(*vCount + 2) * 2] = 1.0f;
        mesh->texcoords[(*vCount + 2) * 2 + 1] = 1.0f;
        mesh->texcoords[(*vCount + 3) * 2] = 0.0f;
        mesh->texcoords[(*vCount + 3) * 2 + 1] = 1.0f;

        // Add indices
        mesh->indices[*iCount] = v1;
        mesh->indices[*iCount + 1] = v2;
        mesh->indices[*iCount + 2] = v3;
        mesh->indices[*iCount + 3] = v1;
        mesh->indices[*iCount + 4] = v3;
        mesh->indices[*iCount + 5] = v4;

        *vCount += 4;
        *iCount += 6;
    }

    void CreateCube(Mesh *mesh, Vector3 position, float size, int *vCount,
                    int *iCount) {
        float hs = size / 2.0f;

        Vector3 p1 = {position.x - hs, position.y - hs, position.z + hs};
        Vector3 p2 = {position.x + hs, position.y - hs, position.z + hs};
        Vector3 p3 = {position.x + hs, position.y + hs, position.z + hs};
        Vector3 p4 = {position.x - hs, position.y + hs, position.z + hs};
        Vector3 p5 = {position.x + hs, position.y - hs, position.z - hs};
        Vector3 p6 = {position.x - hs, position.y - hs, position.z - hs};
        Vector3 p7 = {position.x - hs, position.y + hs, position.z - hs};
        Vector3 p8 = {position.x + hs, position.y + hs, position.z - hs};

        Vector3 n1 = {0.0f, 0.0f, 1.0f};
        AddCubeFace(mesh, p1, p2, p3, p4, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);

        n1 = {0.0f, 0.0f, -1.0f};
        AddCubeFace(mesh, p5, p6, p7, p8, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);

        n1 = {1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p2, p5, p8, p3, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);

        n1 = {-1.0f, 0.0f, 0.0f};
        AddCubeFace(mesh, p6, p1, p4, p7, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);

        n1 = {0.0f, 1.0f, 0.0f};
        AddCubeFace(mesh, p4, p3, p8, p7, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);

        n1 = {0.0f, -1.0f, 0.0f};
        AddCubeFace(mesh, p6, p5, p2, p1, n1, vCount, iCount, 255.0f, 255.0f,
                    255.0f, 255.0f);
    }
};

#endif // CHUNK_H