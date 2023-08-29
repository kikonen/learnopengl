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

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_gbuffer_encode.glsl

void main() {
  Material material = u_materials[fs_in.materialIndex];

  const vec3 normal = normalize(fs_in.normal);

  // combined
  vec4 texColor = material.diffuse;

  o_fragColor = vec4(texColor.xyz, material.ambient);
  o_fragSpecular = material.specular;
  o_fragMetal = material.metal;
  o_fragEmission = texColor.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}
