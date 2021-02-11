#version 330 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec2 texCoords;

  flat int materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} fs_in;

uniform samplerCube skybox;
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

  if (material.diffuse.a < 0.01)
    discard;

  vec3 normal;
  if (materials[matIdx].hasNormalMap) {
    normal = texture(textures[matIdx].normalMap, fs_in.texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    material.hasNormalMap = true;
  } else {
    normal = fs_in.normal;
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  if (texColor.a < 0.1)
    discard;

//  vec3 i = normalize(fs_in.fragPos - viewPos);
//  vec3 r = reflect(i, normal);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  fragColor = texColor;
}
