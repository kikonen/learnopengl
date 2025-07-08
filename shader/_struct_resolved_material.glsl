struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // MRA: [metalness, roughness, ambient-occlusion]
  vec3 mra;

  float ssao;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
