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

uniform Texture textures[MAT_COUNT];

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_tex_resolve_material.glsl
#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl

void main() {
  int matIdx = int(fs_in.materialIndex);
  Material material = resolveMaterial(matIdx);

  if (material.diffuse.a < 0.01)
    discard;

  vec3 normal;
  if (materials[matIdx].hasNormalMap) {
    normal = texture(textures[matIdx].normalMap, fs_in.texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    material.hasNormalMap = true;
  } else {
    normal = normalize(fs_in.normal);
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  if (texColor.a < 0.1)
    discard;

  if (material.hasNormalMap) {
//    texColor = vec4(fs_in.tangentFragPos, 1.0);
  }

//  vec3 i = normalize(fs_in.fragPos - viewPos);
//  vec3 r = reflect(i, normal);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  if (gl_FrontFacing) {
//    texColor = vec4(0.8, 0, 0, 1.0);
  }
  //texColor = vec4(normal, 1.0);
  //texColor = fs_in.fragPosLightSpace;

  fragColor = texColor;
}
