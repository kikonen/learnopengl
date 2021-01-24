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
  vec2 texCoords;

  flat float materialIndex;
  vec3 normal;

  vec4 fragPosLightSpace;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} fs_in;

uniform samplerCube skybox;
uniform sampler2D shadowMap;

uniform Texture textures[MAT_COUNT];

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_tex_dir_light.glsl
#include fn_tex_point_light.glsl
#include fn_tex_spot_light.glsl

void main() {
  int matIdx = int(fs_in.materialIndex);

  bool hasNormalMap = false;
  vec3 norm;
  if (materials[matIdx].hasNormalMap) {
    norm = texture(textures[matIdx].normalMap, fs_in.texCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    hasNormalMap = true;
  } else {
    norm = normalize(fs_in.normal);
  }

  vec3 viewDir = normalize(viewPos - fs_in.fragPos);

  vec4 matAmbient;
  vec4 matDiffuse;
  vec4 matEmission;
  vec4 matSpecular;
  float matShininess;

  {
    if (materials[matIdx].hasDiffuseTex) {
      matDiffuse = texture(textures[matIdx].diffuse, fs_in.texCoords).rgba;
      matAmbient = matDiffuse;
    } else {
      matDiffuse = materials[matIdx].diffuse;
      matAmbient = materials[matIdx].ambient;
    }

    if (materials[matIdx].hasEmissionTex){
      matEmission = texture(textures[matIdx].emission, fs_in.texCoords).rgba;
    }

    if (materials[matIdx].hasSpecularTex){
      matSpecular = texture(textures[matIdx].specular, fs_in.texCoords).rgba;
    } else {
      matSpecular = materials[matIdx].specular;
    }
    matShininess = materials[matIdx].shininess;
  }

  vec4 emission = matEmission;

  bool hasLight = false;
  vec4 dirShaded;
  vec4 pointShaded;
  vec4 spotShaded;

  if (light.use) {
    dirShaded = calculateDirLight(light, norm, viewDir, matAmbient, matDiffuse, matSpecular, matShininess, hasNormalMap, fs_in.fragPosLightSpace);
    hasLight = true;
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (pointLights[i].use) {
      pointShaded += calculatePointLight(pointLights[i], norm, viewDir, fs_in.fragPos, matAmbient, matDiffuse, matSpecular, matShininess, hasNormalMap);
      hasLight = true;
    }
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (spotLights[i].use) {
      spotShaded += calculateSpotLight(spotLights[i], norm, viewDir, fs_in.fragPos, matAmbient, matDiffuse, matSpecular, matShininess, hasNormalMap);
      hasLight = true;
    }
  }

  vec4 shaded = dirShaded + pointShaded + spotShaded + emission;

  vec4 texColor;
  if (hasLight) {
//    if (hasNormalMap) {
//      shaded = shaded + vec4(1.0, 0.0, 0.0, 0.5);
//    }
    texColor = shaded;
  } else {
    texColor = matDiffuse + emission;
  }

  if (texColor.a < 0.1)
    discard;

  if (hasNormalMap) {
//    texColor = vec4(fs_in.tangentFragPos, 1.0);
  }

//  vec3 i = normalize(fs_in.fragPos - viewPos);
//  vec3 r = reflect(i, norm);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  if (gl_FrontFacing) {
//    texColor = vec4(0.8, 0, 0, 1.0);
  }
  //texColor = vec4(normal, 1.0);
  //texColor = fs_in.fragPosLightSpace;

  fragColor = texColor;
}
