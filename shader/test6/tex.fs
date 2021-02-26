#version 450 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec2 texCoords;
  vec3 vertexPos;

  flat int materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  mat3 TBN;
} fs_in;

uniform sampler2D textures[TEX_COUNT];
uniform samplerCube reflectionMap;
uniform samplerCube refractionMap;
uniform sampler2DShadow shadowMap;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl

void main() {
  #include var_tex_material.glsl

  if (material.diffuse.a < 0.01)
    discard;

  vec3 normal;
  if (material.normalMapTex >= 0) {
    normal = texture(textures[material.normalMapTex], fs_in.texCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
  } else {
    normal = fs_in.normal;
  }

  if (material.pattern == 1) {
    normal = calculateNormalPattern(normal);
  }
  if (!gl_FrontFacing) {
    normal = -normal;
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  #include var_calculate_diffuse.glsl

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  if (texColor.a < 0.1)
    discard;

//  texColor = vec4(0.0, 0.8, 0, 1.0);

//  vec3 i = normalize(fs_in.fragPos - viewPos);
//  vec3 r = reflect(i, normal);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  // if (material.normalMapTex >= 0) {
  //   texColor = vec4(normal, 1.0);
  // }

  fragColor = texColor;
}
