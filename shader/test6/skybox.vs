#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 texCoords;

void main() {
  texCoords = aPos;
  vec4 pos = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
  gl_Position = pos.xyww;
}
