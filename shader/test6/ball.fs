#version 450 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl

in VS_OUT {
  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;
} fs_in;

layout (location = 0) out vec4 fragColor;

uniform sampler2DShadow shadowMap;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl

void main() {
  Material material;

  float r = 1.0;//(sin(time) + 1.0) / 2;
  material.diffuse = vec4(r, 1.0, 0.0, 0.25);

  vec3 normal = fs_in.normal;

  vec3 toView = normalize(viewPos - fs_in.fragPos);
  vec4 shaded = calculateLight(normal, toView, material);

  fragColor = shaded;
}
