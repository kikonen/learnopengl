#version 330 core
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  sampler2D diffuseTex;
  sampler2D specularTex;
  sampler2D emissiveTex;
  bool hasDiffuseTex;
  bool hasEmissiveTex;
  bool hasSpecularTex;
};
struct Light {
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  bool use;
};

flat in float texIndex;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;

uniform Material materials[32];

uniform Light light;

out vec4 fragColor;

void main()
{
  int texId = int(texIndex);

  if (light.use) {
    vec3 materialAmbient;
    vec3 materialDiffuse;

    if (materials[texId].hasDiffuseTex) {
      materialDiffuse = texture(materials[texId].diffuseTex, texCoord).rgb;
      materialAmbient = materialDiffuse;
    } else {
      materialDiffuse = materials[texId].diffuse;
      materialAmbient = materials[texId].ambient;
    }

    vec3 emissive;
    if (materials[texId].hasEmissiveTex){
//      emissive = texture(materials[texId].emissiveTex, texCoord).rgb;
    }

    // ambient
    vec3 ambient = light.ambient * materialAmbient;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.pos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = light.diffuse * (diff * materialDiffuse);

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materials[texId].shininess);
    vec3 specular = light.specular * (spec * materials[texId].specular);

    // combined
    vec3 shaded = ambient + diffuse + specular + emissive;

    fragColor = vec4(shaded, 1.0);
  } else {
    fragColor = vec4(materials[texId].diffuse, 1.0);
  }
}
