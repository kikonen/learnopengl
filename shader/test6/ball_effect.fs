#version 450 core

#include constants.glsl

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  vec4 fragPosLightSpace;
} fs_in;

layout (location = 0) out vec4 fragColor;

uniform sampler2D u_textures[TEX_COUNT];
uniform sampler2DShadow u_shadowMap;

int iChannel0;
vec2 iMouse;
vec2 iResolution;
float iTime;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl

//vec3 textureLod(int textureIndex, vec2 texCoord, float dummy)
//{
//  return texture(u_textures[textureIndex], texCoord);
//}

#ifdef EFFECT_SUN
  #include effect_sun.glsl
#endif
#ifdef EFFECT_PLASMA
  #include effect_sun.glsl
#endif

void main() {
  #include var_tex_material.glsl

  iTime = u_time;
  iResolution = u_resolution;
  iMouse = u_resolution * 0.5;
  iChannel0 = material.diffuseTex;

  mainImage(fragColor, gl_FragCoord.xy);

  if (fragColor.r < 0.1 && fragColor.g < 0.1 && fragColor.b < 0.1) {
    fragColor = vec4(fragColor.rgb, 0.0);
  }

#ifdef USE_BLEND
  fragColor = vec4(fragColor.rgb, 0.2);
#endif
}
