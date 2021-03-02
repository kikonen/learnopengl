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
  int dudvMapTex;

  int pattern;
  float reflection;
  float refraction;

  float fogRatio;
  float tiling;
};
