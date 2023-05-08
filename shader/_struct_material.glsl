struct Material {
  vec4 diffuse;
  vec4 emission;
  vec4 specular;

  float ambient;

  int diffuseTex;
  int emissionTex;
  int specularTex;
  int normalMapTex;

  int dudvMapTex;
  int heightMapTex;
  int noiseMapTex;
  int pattern;

  float shininess;
  float reflection;
  float refraction;
  float refractionRatio;

  float tilingX;
  float tilingY;
  int layers;
  float depth;
};
