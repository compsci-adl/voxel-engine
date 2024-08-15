#version 330 core
layout (location = 0) in int vertexPosition;

// out vec2 TexCoord;
out vec3 ourColor;

uniform vec3 inColor;
uniform vec3 worldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
        int offset = 16; // Offset to revert the signed range

    // Decode the 32-bit integer into x, y, z positions
    float x = ((vertexPosition & 0x3F) - offset);         // 6 bits for x
    float y = (((vertexPosition >> 6) & 0x3F) - offset);  // 6 bits for y
    float z = (((vertexPosition >> 12) & 0x3F) - offset); // 6 bits for z
    // No normal or type used in this example for movement
    vec3 decodedPos = vec3(x, y, z);

    gl_Position = projection * view * model * vec4(decodedPos + worldPos, 1.0);
    // TexCoord = vec2(0.0, 0.0); // Assuming no texture coordinates for simplicity
	ourColor = inColor;
}
