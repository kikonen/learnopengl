#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;
} fs_in;

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
#include fn_calculate_fog.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;
  vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);

  vec4 texColor = calculateLight(normal, toView, material);
  texColor = calculateFog(material.fogRatio, texColor);

  fragColor = texColor;
}
