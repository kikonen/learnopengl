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
  int heightMapTex;

  int pattern;

  float reflection;
  float refraction;

  float refractionRatio;

  float fogRatio;

  float tileX;
  float tileY;

  float tilingX;
  float tilingY;

//  int pad1;
//  int pad2;
//  int pad3;
};
