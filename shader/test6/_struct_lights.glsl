struct DirLight {
  vec3 worldPos;

  vec3 worldDir;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};
struct PointLight {
  vec3 worldPos;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct SpotLight {
  vec3 worldPos;
  vec3 worldDir;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;
  float radius;
};
