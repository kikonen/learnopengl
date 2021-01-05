#version 330 core
in vec2 texCoord;
in vec4 color;

uniform vec3 lightColor;

out vec4 fragColor;

void main() {
//  fragColor = vec4(color * lightColor, 1.0f);
  fragColor = color;
}
