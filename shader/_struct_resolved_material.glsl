struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // NOTE KI opacity separated from MRAO
  // MRA: [ambient-occlusion, metalness, roughness]
  vec3 mra;

  float ssao;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
