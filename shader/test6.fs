#version 330 core

#define MAT_COUNT 16
#define LIGHT_COUNT 16

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};
struct DirLight {
  vec3 dir;

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

flat in float texIndex;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;
uniform samplerCube skybox;

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
uniform Material materials[MAT_COUNT];

uniform DirLight light;
uniform PointLight pointLights[LIGHT_COUNT];
uniform SpotLight spotLights[LIGHT_COUNT];

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
  int texId = int(texIndex);
  vec3 norm = normalize(normal);
  vec3 viewDir = normalize(viewPos - fragPos);

  vec4 matAmbient = materials[texId].ambient;
  vec4 matDiffuse = materials[texId].diffuse;
  vec4 matEmission;
  vec4 matSpecular = materials[texId].specular;
  float matShininess = materials[texId].shininess;

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
//  vec3 i = normalize(fragPos - viewPos);
//  vec3 r = reflect(i, norm);
//  texColor = vec4(texture(skybox, r).rgb, 1.0);

  fragColor = texColor;
}

vec4 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(-light.dir);

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
