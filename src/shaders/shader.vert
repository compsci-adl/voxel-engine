#version 330 core
layout (location = 0) in vec3 vertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 worldPos;
uniform vec4 inColor;

out vec4 ourColor;


void main()
{
    gl_Position = projection * view * model * vec4(vertexPosition + worldPos, 1.0f);
    ourColor = inColor;
}