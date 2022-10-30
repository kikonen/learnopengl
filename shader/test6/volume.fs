#version 450 core

#include constants.glsl

#include uniform_matrices.glsl

in VS_OUT {
  vec3 fragPos;
} fs_in;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

void main() {
  fragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
