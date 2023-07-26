// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct DirLight {
  vec4 worldDir;

  vec4 diffuse;
  vec4 specular;
};
struct PointLight {
  vec4 worldPos;

  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct SpotLight {
  vec4 worldPos;
  vec4 worldDir;

  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;
  float radius;
};
