struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;

  bool hasDiffuseTex;
  bool hasEmissionTex;
  bool hasSpecularTex;
  bool hasNormalMap;
};
