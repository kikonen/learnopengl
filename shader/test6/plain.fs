#version 330 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec3 fragPos;

  flat float materialIndex;
  vec3 normal;

  vec4 fragPosLightSpace;
} fs_in;

uniform samplerCube skybox;
uniform sampler2DShadow shadowMap;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_plain_dir_light.glsl
#include fn_plain_point_light.glsl
#include fn_plain_spot_light.glsl

void main() {
  int matIdx = int(fs_in.materialIndex);
  vec3 norm = normalize(fs_in.normal);
  vec3 viewDir = normalize(viewPos - fs_in.fragPos);

  vec4 matAmbient = materials[matIdx].ambient;
  vec4 matDiffuse = materials[matIdx].diffuse;
  vec4 matEmission;
  vec4 matSpecular = materials[matIdx].specular;
  float matShininess = materials[matIdx].shininess;

  vec4 emission = matEmission;

  bool hasLight = false;
  vec4 dirShaded;
  vec4 pointShaded;
  vec4 spotShaded;

  if (light.use) {
    dirShaded = calculateDirLight(light, norm, viewDir, matAmbient, matDiffuse, matSpecular, matShininess, fs_in.fragPosLightSpace);
    hasLight = true;
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (pointLights[i].use) {
      pointShaded += calculatePointLight(pointLights[i], norm, viewDir, fs_in.fragPos, matAmbient, matDiffuse, matSpecular, matShininess);
      hasLight = true;
    }
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (spotLights[i].use) {
      spotShaded += calculateSpotLight(spotLights[i], norm, viewDir, fs_in.fragPos, matAmbient, matDiffuse, matSpecular, matShininess);
      hasLight = true;
    }
  }

  vec4 shaded =  dirShaded + pointShaded + spotShaded + emission;

  vec4 texColor;
  if (hasLight) {
    texColor = shaded;
  } else {
    texColor = matDiffuse + emission;
  }

  if (texColor.a < 0.1)
    discard;

  // reflection test
  float ratio = 1.0 / 1.33;
  vec3 r;
  if (gl_FragCoord.x < 400) {
    r = reflect(-viewDir, norm);
  } else {
    r = refract(-viewDir, norm, ratio);
  }
  texColor = vec4(texture(skybox, r).rgb, 1.0);
//  texColor = vec4(0.0, 0.8, 0, 1.0);

  fragColor = texColor;
}
