#version 460 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

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

Material material;

#include fn_gbuffer_encode.glsl

void main() {
  material = u_materials[fs_in.materialIndex];

  const vec3 normal = normalize(fs_in.normal);

  // combined
  vec4 texColor = material.diffuse;

  o_fragColor = vec4(texColor.xyz, 1.0);
  o_fragMetal = material.metal;
  o_fragEmission = texColor.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}
