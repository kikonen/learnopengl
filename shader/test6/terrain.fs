#version 450 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec3 fragPos;
  vec2 texCoords;

  flat int materialIndex;
  vec3 normal;

  vec4 fragPosLightSpace;
} fs_in;

uniform sampler2DShadow shadowMap;

uniform Texture textures[MAT_COUNT];

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;
  vec3 toView = normalize(viewPos - fs_in.fragPos);

  vec4 texColor = calculateLight(normal, toView, material);

  fragColor = texColor;
}
