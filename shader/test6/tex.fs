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
  vec3 vertexPos;

  flat int materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} fs_in;

uniform bool hasReflectionMap;
uniform samplerCube reflectionMap;

uniform bool hasRefractionMap;
uniform samplerCube refractionMap;

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

  if (hasReflectionMap || hasRefractionMap) {
    float a = 0.25;
    float b = 50.0;
    float x = fs_in.vertexPos.x;
    float y = fs_in.vertexPos.y;
    float z = fs_in.vertexPos.z;
    vec3 N;
    N.x = normal.x + a * sin(b*x);
    N.y = normal.y + a * sin(b*y);
    N.z = normal.z + a * sin(b*z);
    normal = normalize(N);
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  if (hasReflectionMap) {
    vec3 r = reflect(-toView, normal);
    material.diffuse = vec4(texture(reflectionMap, r).rgb, 1.0);
  }

  if (hasRefractionMap) {
    float ratio = 1.0 / 1.33;
    vec3 r = refract(-toView, normal, ratio);
    material.diffuse = vec4(texture(refractionMap, r).rgb, 1.0);
  }

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  if (texColor.a < 0.1)
    discard;

//  texColor = vec4(0.0, 0.8, 0, 1.0);

//  vec3 i = normalize(fs_in.fragPos - viewPos);
//  vec3 r = reflect(i, normal);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  fragColor = texColor;
}
