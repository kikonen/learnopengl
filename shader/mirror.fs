#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec4 glp;

  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;

  mat3 TBN;
} fs_in;

layout(binding = UNIT_MIRROR_REFLECTION) uniform sampler2D u_reflectionTex;

layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap[SHADOW_CASCADES];

layout (location = 0) out vec4 o_fragColor;

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
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);

  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;

  if (!gl_FrontFacing) {
    normal = -normal;
  }

  if (gl_FrontFacing)
  {
    vec4 gp = fs_in.glp;
    vec2 reflectCoord = vec2(-gp.x, gp.y) / (gp.w * 2.0) + 0.5;

    vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

    vec4 mixColor = reflectColor;

    vec4 origDiffuse = material.diffuse;
    material.diffuse = mix(material.diffuse, mixColor, 0.9);
  }

  vec4 color = calculateLight(normal, toView, fs_in.worldPos, fs_in.shadowPos, material);
  color = calculateFog(fs_in.viewPos, material.fogRatio, color);

  o_fragColor = color;
}
