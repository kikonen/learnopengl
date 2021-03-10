struct DirLight {
  vec3 pos;
  bool use;

  vec3 dir;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};
struct PointLight {
  vec3 pos;
  bool use;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct SpotLight {
  vec3 pos;
  bool use;

  vec3 dir;

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
