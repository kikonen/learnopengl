// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct DirLight {
  vec4 worldDir;

  // color + a = intensity
  vec4 diffuse;
};

struct PointLight {
  vec4 worldPos;

  // color + a = intensity
  vec4 diffuse;

  float constant;
  float linear;
  float quadratic;
  float radius;
};

struct SpotLight {
  vec4 worldPos;
  vec4 worldDir;

  // color + a = intensity
  vec4 diffuse;

  float constant;
  float linear;
  float quadratic;

  float cutoff;
  float outerCutoff;
  float radius;
};
