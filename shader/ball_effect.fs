#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
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

uvec2 iChannel0;
vec2 iMouse;
vec2 iResolution;
float iTime;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

#ifdef EFFECT_SUN
  #include effect_sun.glsl
#endif
#ifdef EFFECT_PLASMA
  #include effect_plasma.glsl
#endif

void main() {
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  iTime = u_time;
  iResolution = u_bufferResolution;
  iMouse = u_bufferResolution * 0.5;
  iChannel0 = material.diffuseTex;

  vec4 color;
  mainImage(color, gl_FragCoord.xy);

  clamp_color(color);

  if (color.r < 0.1 && color.g < 0.1 && color.b < 0.1) {
    color = vec4(color.rgb, 0.0);
  }

#ifdef USE_BLEND
  o_fragColor = vec4(color.rgb, 0.2);
#endif
}
