// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Material {
  vec4 diffuse;
  vec4 emission;

  // MRAO: [ambient-occlusion, metalness, roughness, opacity]
  vec4 mrao;

  uvec2 diffuseTex;
  uvec2 emissionTex;
  uvec2 normalMapTex;

  // uvec2 opacityMapTex;
  // MRAO: [ambient-occlusion, metalness, roughness, opacity]
  // - metalness: 0 = dielectric, 1 = metal
  // - roughness: 0 = smooth/shiny, 1 = rough/matte
  // - occlusion: 0 = fully occluded, 1 = no occlusion
  // - opacity:   0 = transparent, 1 = opaque
  uvec2 mraoMapTex;
  uvec2 displacementMapTex;

  uvec2 dudvMapTex;
  uvec2 noiseMapTex;
  uvec2 noise2MapTex;

  uvec2 custom1Tex;

  int pattern;

  float reflection;
  float refraction;
  float refractionRatio;

  float tilingX;
  float tilingY;

  uint spriteCount;
  uint spritesX;
  uint spritesY;

  int layers;
  float layersDepth;
  float parallaxDepth;

  int pad1;
  int pad2;
};
