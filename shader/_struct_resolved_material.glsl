struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // MRAS: [ambient-occlusion, metalness, roughnessa, specular]
  vec4 mras;

  uint flags;

  float ssao;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
