#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aTexIndex;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 model;
uniform mat3 normalMat;

out vec3 color;
flat out float texIndex;
out vec2 texCoord;
out vec3 fragPos;
out vec3 normal;

void main() {
  gl_Position = transform * vec4(aPos, 1.0);

  color = aColor;

  texIndex = aTexIndex;
  texCoord = aTexCoord;

  fragPos = vec3(model * vec4(aPos, 1.0));
  normal = normalMat * aNormal;
}
