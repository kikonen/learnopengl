#version 330 core
in vec2 texCoord;
in vec3 color;
in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 lightColor;

out vec4 fragColor;

void main() {
  fragColor = vec4(color, 1.0);
}
