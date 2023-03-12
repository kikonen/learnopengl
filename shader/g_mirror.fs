#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

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

layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec4 o_fragAmbient;
layout (location = 4) out vec3 o_fragPosition;
layout (location = 5) out vec3 o_fragNormal;

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

  vec4 texColor = material.diffuse;

  o_fragColor = texColor;
  o_fragSpecular = material.specular;
  o_fragSpecular.a = material.shininess;
  o_fragEmission = material.emission;
  o_fragAmbient = material.ambient;

  o_fragPosition = fs_in.worldPos;
  o_fragNormal = normal;
}
