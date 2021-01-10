#version 330 core

#define MAT_COUNT 16
#define LIGHT_COUNT 16

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;

  sampler2D diffuseTex;
  sampler2D emissionTex;
  sampler2D specularTex;
  sampler2D bumpTex;

  bool hasDiffuseTex;
  bool hasEmissionTex;
  bool hasSpecularTex;
  bool hasBumpTex;
};
struct DirLight {
  vec3 dir;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  bool use;
};
struct PointLight {
  vec3 pos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  bool use;
};
struct SpotLight {
  vec3 pos;
  vec3 dir;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;

  bool use;
};

flat in float texIndex;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
uniform Material materials[MAT_COUNT];

uniform DirLight light;
uniform PointLight pointLights[LIGHT_COUNT];
uniform SpotLight spotLights[LIGHT_COUNT];

out vec4 fragColor;

vec3 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess);

vec3 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess);

vec3 calculateSpotLight(
  SpotLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess);


void main() {
  int texId = int(texIndex);
  vec3 norm = normalize(normal);
  vec3 viewDir = normalize(viewPos - fragPos);

  vec3 matAmbient;
  vec3 matDiffuse;
  vec3 matEmission;
  vec3 matSpecular;
  float matShininess;

  {
    if (materials[texId].hasDiffuseTex) {
      matDiffuse = texture(materials[texId].diffuseTex, texCoord).rgb;
      matAmbient = matDiffuse;
    } else {
      matDiffuse = materials[texId].diffuse;
      matAmbient = materials[texId].ambient;
    }

    if (materials[texId].hasEmissionTex){
      matEmission = texture(materials[texId].emissionTex, texCoord).rgb;
    }

    if (materials[texId].hasSpecularTex){
      matSpecular = texture(materials[texId].specularTex, texCoord).rgb;
    } else {
      matSpecular = materials[texId].specular;
    }
    matShininess = materials[texId].shininess;
  }

  vec3 emission = matEmission;

  bool hasLight = false;
  vec3 dirShaded;
  vec3 pointShaded;
  vec3 spotShaded;

  if (light.use) {
    dirShaded = calculateDirLight(light, norm, viewDir, matAmbient, matDiffuse, matSpecular, matShininess);
    hasLight = true;
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (pointLights[i].use) {
      pointShaded += calculatePointLight(pointLights[i], norm, viewDir, fragPos, matAmbient, matDiffuse, matSpecular, matShininess);
      hasLight = true;
    }
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (spotLights[i].use) {
      spotShaded += calculateSpotLight(spotLights[i], norm, viewDir, fragPos, matAmbient, matDiffuse, matSpecular, matShininess);
      hasLight = true;
    }
  }

  vec3 shaded =  dirShaded + pointShaded + spotShaded + emission;

  if (hasLight) {
    fragColor = vec4(shaded, 1.0);
  //    if (materials[texId].hasSpecularTex) {
  //      shaded = shaded + vec3(1.0, 0.0, 0.0);
  //    }
  } else {
    fragColor = vec4(matDiffuse + emission, 1.0);
  }
}

vec3 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(-light.dir);

  // ambient
  vec3 ambient = light.ambient * matAmbient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * matDiffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec3 specular = light.specular * (spec * matSpecular);

  return ambient + diffuse + specular;
}

vec3 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(light.pos - fragPos);

  // ambient
  vec3 ambient = light.ambient * matAmbient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * matDiffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec3 specular = light.specular * (spec * matSpecular);

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}

vec3 calculateSpotLight(
  SpotLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec3 matAmbient,
  vec3 matDiffuse,
  vec3 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(light.pos - fragPos);

  float theta = dot(lightDir, normalize(-light.dir));
  bool shade = theta > light.cutoff;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
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
    vec3 specular = light.specular * (spec * matSpecular);

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
