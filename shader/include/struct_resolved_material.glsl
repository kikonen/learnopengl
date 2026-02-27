struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // MRAS: [metalness, roughnessa, ambient-occlusion, specular]
  vec4 mras;

  uint flags;

  float ssao;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
