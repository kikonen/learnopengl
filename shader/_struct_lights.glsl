struct DirLight {
  vec3 worldDir;
  int pad1;

  vec3 diffuse;
  int pad2;

  vec3 specular;
  int pad3;
};
struct PointLight {
  vec3 worldPos;
  int pad1;

  vec3 diffuse;
  int pad2;

  vec3 specular;
  int pad3;

  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct SpotLight {
  vec3 worldPos;
  int pad1;

  vec3 worldDir;
  int pad2;

  vec3 diffuse;
  int pad3;

  vec3 specular;
  int pad4;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;
  float radius;
};
