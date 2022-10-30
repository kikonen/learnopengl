#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 6) in mat4 a_modelMatrix;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 fragPos;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  gl_Position = u_projectedMatrix * a_modelMatrix * vec4(a_pos, 1.0);
  vs_out.fragPos = (a_modelMatrix * vec4(a_pos, 1.0)).xyz;
}
