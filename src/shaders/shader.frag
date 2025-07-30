#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in float brightness;

uniform sampler2D texture1;

void main()
{
    vec4 textureColour = texture(texture1, texCoord);
    FragColor = vec4(textureColour.rgb * brightness, textureColour.a);
}