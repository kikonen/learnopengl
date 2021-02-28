#version 450 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;
  vec3 vertexPos;

  flat int materialIndex;

  vec4 fragPosLightSpace;

  mat3 TBN;
} fs_in;

uniform sampler2D textures[TEX_COUNT];
uniform samplerCube reflectionMap;
uniform samplerCube refractionMap;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;

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

  vec3 normal = fs_in.normal;

  if (material.pattern == 1) {
    normal = calculateNormalPattern(normal);
  }
  if (!gl_FrontFacing) {
    normal = -normal;
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  #include var_calculate_diffuse.glsl

//  vec4 reflectColor = texture(reflectionTex, fs_in.texCoords).rgba;
//  vec4 refractColor = texture(refractionTex, fs_in.texCoords).rgba;

  vec4 refractColor = texture(refractionTex, (vec2(fs_in.glp.x, fs_in.glp.y)) / (2.0 * fs_in.glp.w) + 0.5);
  vec4 reflectColor = texture(reflectionTex, (vec2(fs_in.glp.x, -fs_in.glp.y)) / (2.0 * fs_in.glp.w) + 0.5);
  vec4 mixColor = (0.2 * refractColor) + (1.0 * reflectColor);
//  mixColor = (1.0 * refractColor);

  material.diffuse = mixColor;

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

//  texColor = vec4(1, 0, 0, 1);

  fragColor = texColor;
}
