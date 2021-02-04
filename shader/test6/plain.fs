#version 330 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  flat float materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} fs_in;

uniform samplerCube skybox;
uniform sampler2DShadow shadowMap;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_plain_resolve_material.glsl
#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl

void main() {
  int matIdx = int(fs_in.materialIndex);
  Material material = resolveMaterial(matIdx);

  vec3 normal = normalize(fs_in.normal);
  vec3 viewDir = normalize(viewPos - fs_in.fragPos);

  vec4 shaded = calculateLight(normal, viewDir, material);
  vec4 texColor = shaded;

  if (texColor.a < 0.1)
    discard;

  // reflection test
  if (false) {
    float ratio = 1.0 / 1.33;
    vec3 r;
    if (gl_FragCoord.x < 400) {
      r = reflect(-viewDir, normal);
    } else {
      r = refract(-viewDir, normal, ratio);
    }
    //texColor = vec4(texture(skybox, r).rgb, 1.0);
  }
//  texColor = vec4(0.0, 0.8, 0, 1.0);

  fragColor = texColor;
}
