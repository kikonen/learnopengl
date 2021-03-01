#version 450 core

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

  flat int materialIndex;

  vec4 fragPosLightSpace;
} fs_in;

uniform sampler2DShadow shadowMap;

uniform sampler2D textures[TEX_COUNT];

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_fog.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;
  vec3 toView = normalize(viewPos - fs_in.fragPos);

  vec4 texColor = calculateLight(normal, toView, material);
  texColor = calculateFog(texColor);

  fragColor = texColor;
}
