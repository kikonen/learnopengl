#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
out float colorW;
out vec4 color;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  texCoords = aTexCoords;
  color = vec4(aColor, 1.0f);
  colorW = abs(gl_Position.w);
}
