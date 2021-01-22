#version 330 core

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
#define MAT_COUNT 8
#define LIGHT_COUNT 8

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;

  bool hasDiffuseTex;
  bool hasEmissionTex;
  bool hasSpecularTex;
  bool hasNormalMap;
};

struct DirLight {
  vec4 dir;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  bool use;
};
struct PointLight {
  vec3 pos;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;

  bool use;
};
struct SpotLight {
  vec3 pos;
  vec3 dir;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;

  bool use;
};

in VS_OUT {
  vec3 fragPos;

  flat float materialIndex;
  vec3 normal;
} fs_in;

layout (std140) uniform Data {
  vec3 viewPos;
  float time;
};
uniform samplerCube skybox;

layout (std140) uniform Materials {
  Material materials[MAT_COUNT];
};

layout(std140) uniform Lights {
  DirLight light;
  PointLight pointLights[LIGHT_COUNT];
  SpotLight spotLights[LIGHT_COUNT];
};

out vec4 fragColor;

vec4 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess);

vec4 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess);

vec4 calculateSpotLight(
  SpotLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess);


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
    dirShaded = calculateDirLight(light, norm, viewDir, matAmbient, matDiffuse, matSpecular, matShininess);
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

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

vec4 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(-vec3(light.dir));

  // ambient
  vec4 ambient = light.ambient * matAmbient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec4 diffuse = light.diffuse * (diff * matDiffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec4 specular = light.specular * (spec * matSpecular);

  return ambient + diffuse + specular;
}

vec4 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(light.pos - fragPos);

  // ambient
  vec4 ambient = light.ambient * matAmbient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec4 diffuse = light.diffuse * (diff * matDiffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec4 specular = light.specular * (spec * matSpecular);

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}

vec4 calculateSpotLight(
  SpotLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(light.pos - fragPos);

  float theta = dot(lightDir, normalize(-light.dir));
  bool shade = theta > light.cutoff;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  if (shade) {
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // ambient
    ambient = light.ambient * matAmbient;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    diffuse = light.diffuse * (diff * matDiffuse);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
    vec4 specular = light.specular * (spec * matSpecular);

    diffuse  *= intensity;
    specular *= intensity;
  } else {
    ambient = light.ambient * matDiffuse;
  }

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}
