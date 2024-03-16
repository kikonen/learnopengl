#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec4 glp;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
} fs_in;

layout(binding = UNIT_MIRROR_REFLECTION) uniform sampler2D u_reflectionTex;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_calculate_normal_pattern.glsl
#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  vec3 normal = normalize(fs_in.normal);

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

  if (gl_FrontFacing)
  {
    vec2 ndc = (fs_in.glp.xy / fs_in.glp.w) / 2.0 + 0.5;

    vec2 reflectCoord = vec2(-ndc.x, ndc.y);

    vec4 reflectColor = textureLod(u_reflectionTex, reflectCoord, 0);

    vec4 mixColor = reflectColor;

    material.diffuse = mix(material.diffuse, mixColor, 0.9);
  }

  vec4 color = material.diffuse;

  clamp_color(color);

  o_fragColor = color.xyz;
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}
