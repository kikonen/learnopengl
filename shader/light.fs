#version 460 core

#include struct_material.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl

in VS_OUT {
  flat uint entityIndex;

  flat uint materialIndex;

  vec2 texCoord;
  vec3 worldPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

void main() {
  material = u_materials[fs_in.materialIndex];

  // combined
  vec4 texColor = material.diffuse;

  o_fragColor = texColor;
}
