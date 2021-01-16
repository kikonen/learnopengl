#version 330 core
in vec2 texCoords;
in vec4 color;
in float colorW;

out vec4 fragColor;

void main() {
  fragColor = vec4(color.x, color.y, color.z, 1.0f);
}
