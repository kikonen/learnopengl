#version 450 core

#include constants.glsl

layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 aModelMatrix;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 fragPos;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  gl_Position = u_projectedMatrix * aModelMatrix * vec4(aPos, 1.0);
  vs_out.fragPos = (aModelMatrix * vec4(aPos, 1.0)).xyz;
}
