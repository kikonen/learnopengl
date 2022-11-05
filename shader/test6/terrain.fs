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
  vec2 texCoords;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;
} fs_in;

uniform sampler2DShadow u_shadowMap;

uniform sampler2D u_textures[TEX_COUNT];

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_fog.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;
  vec3 toView = normalize(u_viewPos - fs_in.fragPos);

  vec4 texColor = calculateLight(normal, toView, material);
  texColor = calculateFog(material.fogRatio, texColor);

  fragColor = texColor;
}
