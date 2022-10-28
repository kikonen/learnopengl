#version 450 core

#include constants.glsl

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 6) in mat4 aModelMatrix;

#include uniform_matrices.glsl
#include uniform_data.glsl

flat out vec4 color;

void main()
{
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);
  color = aColor;
}
