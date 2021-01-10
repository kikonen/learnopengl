#version 330 core
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  sampler2D diffuseTex;
  sampler2D emissionTex;
  sampler2D specularTex;
  bool hasDiffuseTex;
  bool hasemissionTex;
  bool hasSpecularTex;
};
struct Light {
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
  bool directional;
  bool point;
  bool spot;
};

flat in float texIndex;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
uniform Material materials[16];

uniform Light light;

out vec4 fragColor;

void main()
{
  int texId = int(texIndex);

  vec3 materialAmbient;
  vec3 materialDiffuse;

  if (materials[texId].hasDiffuseTex) {
    materialDiffuse = texture(materials[texId].diffuseTex, texCoord).rgb;
    materialAmbient = materialDiffuse;
  } else {
    materialDiffuse = materials[texId].diffuse;
    materialAmbient = materials[texId].ambient;
  }

  if (light.use) {
    vec3 emission;
    if (materials[texId].hasemissionTex){
      emission = vec3(texture(materials[texId].emissionTex, texCoord));
    }

    vec3 norm = normalize(normal);

    vec3 lightDir;
    if (light.directional) {
      lightDir = normalize(-light.dir);
    } else {
      lightDir = normalize(light.pos - fragPos);
    }

    bool shade = true;
    float intensity;
    if (light.spot) {
      float theta = dot(lightDir, normalize(-light.dir));
      shade = theta > light.cutoff;

      if (shade) {
        float epsilon = light.cutoff - light.outerCutoff;
        intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
      }
    }

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if (shade) {
      // ambient
      ambient = light.ambient * materialAmbient;

      // diffuse
      float diff = max(dot(norm, lightDir), 0.0);
      diffuse = light.diffuse * (diff * materialDiffuse);

      // specular
      vec3 viewDir = normalize(viewPos - fragPos);
      vec3 reflectDir = reflect(-lightDir, norm);

      float spec = pow(max(dot(viewDir, reflectDir), 0.0), materials[texId].shininess);

      if (materials[texId].hasSpecularTex){
        specular = light.specular * (spec * texture(materials[texId].specularTex, texCoord).rgb);
      } else {
        specular = light.specular * (spec * materials[texId].specular);
      }

      if (light.spot) {
        diffuse  *= intensity;
        specular *= intensity;
      }
    } else {
      ambient = light.ambient * materialDiffuse;
    }

    if (light.point) {
      float distance = length(light.pos - fragPos);
      float attenuation = 1.0 / (light.constant + light.linear * distance +
                                 light.quadratic * (distance * distance));
      ambient  *= attenuation;
      diffuse  *= attenuation;
      specular *= attenuation;
    }

    // combined
    vec3 shaded = ambient + diffuse + specular + emission;
//    if (materials[texId].hasSpecularTex) {
//      shaded = shaded + vec3(1.0, 0.0, 0.0);
//    }

    fragColor = vec4(shaded, 1.0);
  } else {
    fragColor = vec4(materialDiffuse, 1.0);
  }
}
