struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 emission;
  vec4 specular;
  float shininess;

  int diffuseTex;
  int emissionTex;
  int specularTex;
  int normalMapTex;

  int pattern;
  bool reflection;
  bool refraction;
};
