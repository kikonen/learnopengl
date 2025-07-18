#version 460 core

#include ssbo_materials.glsl

#include uniform_data.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  flat uint materialIndex;

  vec2 texCoord;
  vec3 viewPos;
  vec3 normal;
} fs_in;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  const vec3 normal = normalize(fs_in.normal);

  // combined
  vec4 texColor = material.diffuse;

  o_fragColor = texColor.rgb;
  o_fragMRA = material.mra;
  o_fragEmission = texColor.rgb;

  #include encode_gbuffer_normal.glsl
  #include encode_gbuffer_view_position.glsl
  #include encode_gbuffer_view_z.glsl
}
