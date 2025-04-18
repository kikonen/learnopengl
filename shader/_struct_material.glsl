// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Material {
  vec4 diffuse;
  vec4 emission;

  // MRAO: [metalness, roughness, ambient-occlusion, opacity]
  vec4 mrao;

  uvec2 diffuseTex;
  uvec2 emissionTex;
  uvec2 normalMapTex;

  // uvec2 opacityMapTex;
  // MRAO: [metalness, roughness, ambient-occlusion, opacity]
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
