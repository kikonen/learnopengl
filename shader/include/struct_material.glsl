#define MATERIAL_INVERT_OCCLUSION 1
#define MATERIAL_INVERT_METALNESS 2
#define MATERIAL_INVERT_ROUGHNESS 4
// tilingX/Y are "tiles per world unit"; multiply by entity world scale
#define MATERIAL_SCALE_TILING 8

// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct MaterialMain {
  vec4 diffuse;
  vec4 emission;

  // MRAS: [metalness, roughness, ambient-occlusion, specular]
  vec4 mras;

  uint diffuseTex;
  uint emissionTex;
  uint normalMapTex;

  // uint opacityMapTex;

  // MRAS: [metalness, roughness, ambient-occlusion, specular]
  // - metalness (Red):   0 = dielectric, 1 = metal
  // - roughness (Green): 0 = smooth/shiny, 1 = rough/matte
  // - occlusion (Blue):  0 = fully occluded, 1 = no occlusion
  // - specular  (Alpha): 0 = no reflection, 1 = strong reflection
  uint mrasMapTex;

  uint flags;

  float tilingX;
  float tilingY;

  float parallaxDepth;

  // int pad3_1;
  // int pad3_2;
  // int pad3_3;
};

struct MaterialCustom {
  uint displacementMapTex;
  uint dudvMapTex;
  uint noiseMapTex;
  uint noise2MapTex;

  uint custom1Tex;

  uint fontAtlasTex;

  uint dynamicTex;

  float dynamicRatio;

  // int pad3_1;
  // int pad3_2;
  // int pad3_3;
};

struct MaterialCold {
  float reflection;
  float refraction;
  float refractionRatio;

  uint packedSprites;

  int layers;
  float layersDepth;
  float pointSize;

  int pad3_1;
  // int pad3_2;
  // int pad3_3;
};

// packed = spriteCount(16) | spritesX(8) | spritesY(8)
// Must match the C++ packing in Material upload — keep bit layout in sync.
uint packSprites(
  uint count,
  uint spritesPerRow,
  uint spritesX,
  uint spritesY) {
  return ((count & 0xFFu) << 24) |
    ((spritesPerRow & 0xFFu) << 16) |
    ((spritesX & 0xFFu) << 8) |
    (spritesY & 0xFFu);
}

// GPU-side sprite bounds check;
// unused until particle logic moves to GPU (currently clamped CPU-side)
uint unpacSpriteCount(uint bits)   { return (bits >> 24) & 0xFFu; }
uint unpacSpritesPerRow(uint bits) { return (bits >> 16) & 0xFFu; }
uint unpackSpritesX(uint bits)     { return (bits >> 8)  & 0xFFu; }
uint unpackSpritesY(uint bits)     { return  bits        & 0xFFu; }
