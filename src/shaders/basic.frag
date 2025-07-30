#version 330 core
in vec3 colour;
in float brightness;

out vec4 finalColor;


void main() {
    finalColor = vec4(colour * brightness, 1.0f);
}