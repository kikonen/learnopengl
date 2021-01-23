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
