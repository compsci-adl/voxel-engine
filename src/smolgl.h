#ifndef SMOLGL_H
#define SMOLGL_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>

#ifndef SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define SMOLGL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION 0
#endif
#include <stdio.h>

#ifndef SMOLGL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#define SMOLGL_DEFAULT_SHADER_ATTRIB_NAME_POSITION                             \
    "vertexPosition" // Bound by default to shader location:
                     // SMOLGL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#endif

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
float cameraSpeedMultiplier = 20.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0
                    // results in a direction vector pointing to the right so we
                    // initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 90.0f;

unsigned int smolLoadVertexBuffer(const void *buffer, int size, bool dynamic) {
    unsigned int id = 0;

    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer,
                 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    return id;
}

// Set vertex attribute
void smolSetVertexAttribute(unsigned int index, int compSize, int type,
                            bool normalized, int stride, int offset) {
    // NOTE: Data type could be: GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT,
    // GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT Additional types (depends on
    // OpenGL version or extensions):
    //  - GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED,
    //  - GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV,
    //  GL_UNSIGNED_INT_10F_11F_11F_REV

    size_t offsetNative = offset;
    glVertexAttribPointer(index, compSize, type, normalized, stride,
                          (void *)offsetNative);
}

void smolEnableVertexAttribute(unsigned int index) {
    glEnableVertexAttribArray(index);
}

// Load a new attributes element buffer
unsigned int smolLoadVertexBufferElement(const void *buffer, int size,
                                         bool dynamic) {
    unsigned int id = 0;

    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer,
                 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    return id;
}

void smolUnloadVertexArray(unsigned int vaoId) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vaoId);
    printf("VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)\n", vaoId);
}

void smolDrawVertexArrayElements(int offset, int count, const void *buffer) {
    // NOTE: Added pointer math separately from function to avoid UBSAN
    // complaining
    unsigned int *bufferPtr = (unsigned int *)buffer;
    if (offset > 0)
        bufferPtr += offset;

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT,
                   (const unsigned int *)bufferPtr);
}

void smolDrawVertexArray(int offset, int count) {
    glDrawArrays(GL_TRIANGLES, offset, count);
}

#endif // SMOLGL_H