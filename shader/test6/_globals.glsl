#define UBO_MATRICES 0
#define UBO_DATA 1
#define UBO_CLIP_PLANES 2
#define UBO_LIGHTS 3
#define UBO_MATERIALS 4
#define UBO_MATERIAL 5
#define UBO_TEXTURES 6

#define UNIT_WATER_NOISE 64
#define UNIT_WATER_REFLECTION 65
#define UNIT_WATER_REFRACTION 66
#define UNIT_MIRROR_REFLECTION 67
//#define UNIT_MIRROR_RERACTION 68
#define UNIT_CUBE_MAP 69
#define UNIT_SHADOW_MAP 70
#define UNIT_SKYBOX 71
#define UNIT_VIEWPORT 72

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
// => defined by c++ side (more optimal)
#ifndef MAT_COUNT
  #define MAT_COUNT 8
#endif

// NOTE KI TEX_COUNT == texture index != texture unit index
// => i.e. unit index is not 1:1 mapping to texture index
#ifndef TEX_COUNT
  #define TEX_COUNT 256
#endif

#ifndef CLIP_COUNT
  #define CLIP_COUNT 2
#endif

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
#ifndef LIGHT_COUNT
  #define LIGHT_COUNT 8
#endif
