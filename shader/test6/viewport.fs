#version 450 core
const int EFF_NONE = 0;
const int EFF_INVERT = 1;
const int EFF_GRAY_SCALE = 2;

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D viewportTexture;
uniform int effect;

void main()
{
  vec4 color = texture(viewportTexture, texCoords).rgba;

  if (effect == EFF_INVERT) {
    color = vec4(vec3(1.0 - color), color.a);
  } else if (effect == EFF_GRAY_SCALE) {
    float avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(avg, avg, avg, color.a);
  }

  fragColor = color;
}
