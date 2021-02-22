#version 430 core
out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D particleTexture;

void main()
{
  fragColor = texture(particleTexture, texCoords).rgba;
}
