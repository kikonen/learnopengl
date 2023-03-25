struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 emission;
  vec4 specular;

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
  float fogRatio;
  float tilingX;
  float tilingY;

  int layers;
  float depth;

  int pad1;
  int pad2;
  int pad3;
};
