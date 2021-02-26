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

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;

  if (material.pattern == 1) {
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
  if (!gl_FrontFacing) {
    normal = -normal;
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

  if (material.reflection) {
    vec3 r = reflect(-toView, normal);
    material.diffuse = vec4(texture(reflectionMap, r).rgb, 1.0);
  }

  if (material.refraction) {
    float ratio = 1.0 / 1.33;
    vec3 r = refract(-toView, normal, ratio);
    material.diffuse = vec4(texture(refractionMap, r).rgb, 1.0);
  }

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  fragColor = texColor;
}
