#version 430 core
out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D viewportTexture;

void main()
{
  vec4 color = texture(viewportTexture, texCoords).rgba;
  fragColor = color;
}
