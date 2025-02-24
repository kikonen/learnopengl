#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  vec4 shadowPos;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

uvec2 iChannel0;
vec2 iMouse;
vec2 iResolution;
float iTime;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#ifdef EFFECT_SUN
  #include effect_sun.glsl
#endif
#ifdef EFFECT_PLASMA
  #include effect_plasma.glsl
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  iTime = u_time;
  iResolution = u_bufferResolution;
  iMouse = u_bufferResolution * 0.5;
  iChannel0 = u_materials[materialIndex].diffuseTex;

  vec4 color;
  mainImage(color, gl_FragCoord.xy);

  clamp_color(color);

  if (color.r < 0.1 && color.g < 0.1 && color.b < 0.1) {
    color = vec4(color.rgb, 0.0);
  }

#ifdef USE_BLEND
  o_fragColor = vec4(color.rgb, 0.2);
#else
  o_fragColor = vec4(color.rgb, 1.0);
#endif
}
