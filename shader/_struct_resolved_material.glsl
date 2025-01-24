struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // MRAO: [metalness, roughness, ambient-occlusion]
  vec3 mrao;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
