struct DirLight {
  vec3 worldDir;

  vec3 diffuse;
  vec3 specular;
};
struct PointLight {
  vec3 worldPos;

  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct SpotLight {
  vec3 worldPos;
  vec3 worldDir;

  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;
  float radius;
};
