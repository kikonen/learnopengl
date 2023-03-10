#version 460 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
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

in TES_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;
#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec3 o_fragPosition;
layout (location = 2) out vec3 o_fragNormal;
layout (location = 3) out vec4 o_fragEmission;

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
  const Entity entity = u_entities[fs_in.entityIndex];
  #include var_tex_material.glsl

#ifdef USE_ALPHA
  if (material.diffuse.a < 0.01)
    discard;
#endif

  #include var_tex_material_normal.glsl

  if (material.pattern == 1) {
    normal = calculateNormalPattern(normal);
  }

  if (!gl_FrontFacing) {
    normal = -normal;
  }

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

  if (!gl_FrontFacing) {
    float alpha = texColor.a;
    texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
    texColor.a = alpha;
  }

  texColor = calculateFog(material.fogRatio, texColor);

  sampler2D heightMap = sampler2D(u_texture_handles[material.heightMapTex]);
  float h = texture(heightMap, fs_in.texCoord).r;

//  texColor = vec4(h, h, h, 1.0);

  o_fragColor = texColor;
  o_fragPosition = fs_in.worldPos;
  o_fragNormal = fs_in.normal;
}
