#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  vec4 shadowPos;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

int iChannel0;
vec2 iMouse;
vec2 iResolution;
float iTime;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#ifdef EFFECT_SUN
  #include effect_sun.glsl
#endif
#ifdef EFFECT_PLASMA
  #include effect_plasma.glsl
#endif

void main() {
  #include var_tex_material.glsl

  iTime = u_time;
  iResolution = u_resolution;
  iMouse = u_resolution * 0.5;
  iChannel0 = material.diffuseTex;

  mainImage(o_fragColor, gl_FragCoord.xy);

  if (o_fragColor.r < 0.1 && o_fragColor.g < 0.1 && o_fragColor.b < 0.1) {
    o_fragColor = vec4(o_fragColor.rgb, 0.0);
  }

#ifdef USE_BLEND
  o_fragColor = vec4(o_fragColor.rgb, 0.2);
#endif
}
