#version 330 core
in vec2 texCoords;
in vec4 color;

uniform sampler2D texture1;

out vec4 fragColor;

void main() {
  fragColor = texture(texture1, texCoords);
}
