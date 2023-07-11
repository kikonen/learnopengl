struct Material {
  vec4 diffuse;
  vec3 emission;

  // specular + shininess
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

  float reflection;
  float refraction;
  float refractionRatio;

  float tilingX;
  float tilingY;
  int layers;
  float depth;
};
