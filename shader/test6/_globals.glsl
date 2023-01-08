#define UBO_MATRICES 0
#define UBO_DATA 1
#define UBO_CLIP_PLANES 2
#define UBO_LIGHTS 3
//#define UBO_MATERIALS 4
#define UBO_TEXTURES 5

#define SSBO_MATERIALS 1
//#define SSBO_TEXTURES 2
#define SSBO_ENTITIES 3
#define SSBO_MATERIAL_INDECES 4

#define UNIT_WATER_NOISE 64
#define UNIT_WATER_REFLECTION 65
#define UNIT_WATER_REFRACTION 66
#define UNIT_MIRROR_REFLECTION 67
//#define UNIT_MIRROR_RERACTION 68
#define UNIT_CUBE_MAP 69
#define UNIT_SHADOW_MAP 70
#define UNIT_SKYBOX 71
#define UNIT_VIEWPORT 72

#define ATTR_POS 0
#define ATTR_NORMAL 1
#define ATTR_TANGENT 2
//#define ATTR_BITANGENT 3
//#define ATTR_MATERIAL_INDEX 4
//#define ATTR_INSTANCE_MATERIAL_INDEX ATTR_MATERIAL_INDEX
#define ATTR_TEX 4

#define ATTR_INSTANCE_ENTITY_INDEX 5

// #define ATTR_INSTANCE_MODEL_MATRIX_1 6
// #define ATTR_INSTANCE_MODEL_MATRIX_2 7
// #define ATTR_INSTANCE_MODEL_MATRIX_3 8
// #define ATTR_INSTANCE_MODEL_MATRIX_4 9

// #define ATTR_INSTANCE_NORMAL_MATRIX_1 10
// #define ATTR_INSTANCE_NORMAL_MATRIX_2 11
// #define ATTR_INSTANCE_NORMAL_MATRIX_3 12

// #define ATTR_INSTANCE_OBJECT_ID 13
// #define ATTR_INSTANCE_HIGHLIGHT_INDEX 14

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
// => defined by c++ side (more optimal)
// #ifndef MAT_COUNT
//   #define MAT_COUNT 200
// #endif

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
