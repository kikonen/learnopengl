struct Material {
  vec4 diffuse;
  vec3 specular;
  vec3 emission;

  float ambient;

  int diffuseTex;
  int specularTex;
  int emissionTex;
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
