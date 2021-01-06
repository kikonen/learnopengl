#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 model;

out vec2 texCoord;
out vec3 color;
out vec3 normal;
out vec3 fragPos;

void main() {
  gl_Position = transform * vec4(aPos, 1.0);
  texCoord = aTexCoord;
  color = aColor;

  fragPos = vec3(model * vec4(aPos, 1.0));
  normal = aNormal;
}
