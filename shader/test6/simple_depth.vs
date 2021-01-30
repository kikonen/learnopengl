#version 330 core
layout (location = 0) in vec3 aPos;

#include uniform_matrices.glsl

uniform mat4 modelMatrix;

void main()
{
  gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}
