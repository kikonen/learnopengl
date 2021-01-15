#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aTexIndex;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aNormal;

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};

//uniform mat4 transform;
uniform mat4 model;

out vec3 fragPos;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);

  fragPos = vec3(model * vec4(aPos, 1.0));
}
