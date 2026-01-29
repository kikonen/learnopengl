#define MATERIAL_INVERT_OCCLUSION 1
#define MATERIAL_INVERT_METALNESS 2
#define MATERIAL_INVERT_ROUGHNESS 4

// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Material {
  vec4 diffuse;
  vec4 emission;

  // MRAS: [ambient-occlusion, metalness, roughness, specular]
  vec4 mras;

  uvec2 diffuseTex;
  uvec2 emissionTex;
  uvec2 normalMapTex;

  uvec2 opacityMapTex;

  // MRAS: [ambient-occlusion, metalness, roughness, specular]
  // - occlusion: 0 = fully occluded, 1 = no occlusion
  // - metalness: 0 = dielectric, 1 = metal
  // - roughness: 0 = smooth/shiny, 1 = rough/matte
  // - specular:  0 = no reflection, 1 = strong reflection
  uvec2 mrasMapTex;

  uvec2 displacementMapTex;

  uvec2 dudvMapTex;
  uvec2 noiseMapTex;
  uvec2 noise2MapTex;

  uvec2 custom1Tex;

  uint flags;

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

  // int pad1;
  // int pad2;
};
