#version 330 core
out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D viewportTexture;

void main()
{
  vec3 color = texture(viewportTexture, texCoords).rgb;
  fragColor = vec4(color, 1.0);
}
