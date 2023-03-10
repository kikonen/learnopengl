#version 460 core

#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_matrices.glsl

in VS_OUT {
  vec3 worldPos;
  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  Material material = u_materials[fs_in.materialIndex];

  o_fragColor = material.diffuse;
}
