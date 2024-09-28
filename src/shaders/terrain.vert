#version 330 core
layout (location = 0) in int vertexPosition;

// out vec2 TexCoord;
out vec3 ourColor;

uniform vec3 inColor;
uniform bool useInColor;
uniform vec3 worldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const vec3 colors[3] = vec3[](vec3(0.0, 0.0, 0.0), vec3(0.0, 0.5, 0.0), vec3(0.5, 0.5, 0.0));

void main()
{
    int offset = 16; // Offset to revert the signed range

    // Decode the 32-bit integer into x, y, z positions
    float x = ((vertexPosition & 0x3F) - offset);         // 6 bits for x
    float y = (((vertexPosition >> 6) & 0x3F) - offset);  // 6 bits for y
    float z = (((vertexPosition >> 12) & 0x3F) - offset); // 6 bits for z
    int colorPos = (((vertexPosition >> 21) & 0x3F)); // 6 bits for texture
    // No normal or type used in this example for movement
    vec3 decodedPos = vec3(x, y, z);

    gl_Position = projection * view * model * vec4(decodedPos + worldPos, 1.0);
    // TexCoord = vec2(0.0, 0.0); // Assuming no texture coordinates for simplicity
    if (!useInColor) {
        ourColor = colors[colorPos];
    } else {
        ourColor = inColor;
    }
}
