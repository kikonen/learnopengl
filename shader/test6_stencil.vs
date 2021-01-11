#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aTexIndex;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 model;

out vec3 fragPos;

void main() {
//  gl_Position = transform * vec4(aPos, 1.0);
  gl_Position = transform * vec4(aPos + aNormal * 0.02, 1.0);

  fragPos = vec3(model * vec4(aPos, 1.0));
}
