#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

uniform mat4 transform;

out vec2 texCoord;
out vec4 color;

void main() {
  gl_Position = transform * vec4(aPos, 1.0);
  texCoord = aTexCoord;
  color = vec4(aColor, 1.0f);
}
