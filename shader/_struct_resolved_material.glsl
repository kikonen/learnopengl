struct ResolvedMaterial {
  vec4 diffuse;
  vec3 emission;

  // G buffer: metalness, roughness, displacement, ambient-occlusion
  vec4 metal;

  float reflection;
  float refraction;
  float refractionRatio;

  float parallaxDepth;
};
