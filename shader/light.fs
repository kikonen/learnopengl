#version 460 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

in VS_OUT {
  flat uint materialIndex;
  vec2 texCoord;
  vec3 worldPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  Material material = u_materials[fs_in.materialIndex];

  // combined
  vec4 texColor = material.diffuse;

  fragColor = texColor;
}
