#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in GS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;
#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

void main() {
  #include var_tex_material.glsl

#ifdef USE_ALPHA
  if (material.diffuse.a < 0.01)
    discard;
#endif

  #include var_tex_material_normal.glsl

  vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);

  #include var_calculate_diffuse.glsl

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

#ifdef USE_ALPHA
  if (texColor.a < 0.1)
    discard;
#endif

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  texColor = calculateFog(material.fogRatio, texColor);
  fragColor = texColor;
}