struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // MRAS: [metalness, roughnessa, ambient-occlusion, specular]
  vec4 mras;

  float ssao;

  uint flags;

  float reflection;
  float refraction;
  float refractionRatio;

  // float reflection;
  // float refraction;
  // float refractionRatio;

  float tilingX;
  float tilingY;

  uint spriteCount;
  uint spritesX;
  uint spritesY;

  int layers;
  float layersDepth;
  float parallaxDepth;
};
