#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec3 triOffset;

void main() {
  gl_Position = vec4(triOffset.x + aPos.x, triOffset.y + aPos.y, triOffset.z + aPos.z, 1.0);
}
