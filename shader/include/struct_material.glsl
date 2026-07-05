#define MATERIAL_INVERT_OCCLUSION 1
#define MATERIAL_INVERT_METALNESS 2
#define MATERIAL_INVERT_ROUGHNESS 4

// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Material {
  vec4 diffuse;
  vec4 emission;

  // MRAS: [metalness, roughness, ambient-occlusion, specular]
  vec4 mras;

  uvec2 diffuseTex;
  uvec2 emissionTex;
  uvec2 normalMapTex;

  uvec2 opacityMapTex;

  // MRAS: [metalness, roughness, ambient-occlusion, specular]
  // - metalness (Red):   0 = dielectric, 1 = metal
  // - roughness (Green): 0 = smooth/shiny, 1 = rough/matte
  // - occlusion (Blue):  0 = fully occluded, 1 = no occlusion
  // - specular  (Alpha): 0 = no reflection, 1 = strong reflection
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

  uint packedSprites;

  int layers;
  float layersDepth;
  float parallaxDepth;
  float pointSize;

  int pad3_1;
  // int pad3_2;
  // int pad3_3;
};

// packed = spriteCount(16) | spritesX(8) | spritesY(8)
// Must match the C++ packing in Material upload — keep bit layout in sync.
uint packSprites(uint count, uint spritesX, uint spritesY) {
  return (count << 16) | ((spritesX & 0xFFu) << 8) | (spritesY & 0xFFu);
}

// GPU-side sprite bounds check;
// unused until particle logic moves to GPU (currently clamped CPU-side)
uint unpacSpriteCount(uint bits) { return  bits >> 16; }
uint unpackSpritesX(uint bits)    { return (bits >> 8) & 0xFFu; }
uint unpackSpritesY(uint bits)    { return  bits        & 0xFFu; }
