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
  #define LIGHT_COUNT 128
#endif

#define MAX_SHADOW_MAP_COUNT_ABS 5
#ifndef MAX_SHADOW_MAP_COUNT
  #define MAX_SHADOW_MAP_COUNT 4
#endif

#ifndef CS_GROUP_X
  #define CS_GROUP_X 1
#endif

#ifndef CS_GROUP_Y
  #define CS_GROUP_Y 1
#endif

// ##########
#define UBO_MATRICES 0
#define UBO_DATA 1
#define UBO_CLIP_PLANES 2
#define UBO_LIGHTS 3
//#define UBO_MATERIALS 4
#define UBO_TEXTURES 5
#define UBO_BUFFER_INFO 6

#define SSBO_MATERIALS 1
//#define SSBO_TEXTURES 2
#define SSBO_ENTITIES 3
#define SSBO_MATERIAL_INDECES 4
#define SSBO_DRAW_COMMANDS 5
#define SSBO_DRAW_PARAMETERS 6
#define SSBO_PERFORMANCE_COUNTERS 7
#define SSBO_SHAPES 8
#define SSBO_INSTANCE_INDECES 9

#define UNIFORM_PROJECTION_MATRIX 1
#define UNIFORM_VIEW_MATRIX 2
#define UNIFORM_NEAR_PLANE 3
#define UNIFORM_FAR_PLANE 4
#define UNIFORM_DRAW_PARAMETERS_INDEX 6
#define UNIFORM_STENCIL_MODE 7
#define UNIFORM_SHADOW_MAP_INDEX 8
#define UNIFORM_EFFECT_BLOOM_ITERATION 9

#define UNIFORM_TONE_HDRI 10
#define UNIFORM_GAMMA_CORRECT 11
#define UNIFORM_VIEWPORT_TRANSFORM 12

#define UNIFORM_MODEL_MATRIX 13
#define UNIFORM_MATERIAL_INDEX 14

#define SUBROUTINE_EFFECT 0

#define UNIT_FONT_ATLAS 48

#define UNIT_HDR_TEXTURE 49

#define UNIT_G_ALBEDO 50
//#define UNIT_G_SPECULAR 51
#define UNIT_G_EMISSION 52
//#define UNIT_G_POSITION 53
#define UNIT_G_METAL 53
#define UNIT_G_NORMAL 54
#define UNIT_G_DEPTH 55

#define UNIT_OIT_ACCUMULATOR 56
#define UNIT_OIT_REVEAL 57

#define UNIT_EFFECT_ALBEDO 58
#define UNIT_EFFECT_BRIGHT 59
#define UNIT_EFFECT_WORK 60

#define UNIT_WATER_NOISE 64
#define UNIT_WATER_REFLECTION 65
#define UNIT_WATER_REFRACTION 66
#define UNIT_MIRROR_REFLECTION 67
#define UNIT_CUBE_MAP 68
#define UNIT_SKYBOX 69

#define UNIT_ENVIRONMENT_MAP 70
#define UNIT_IRRADIANCE_MAP 71
#define UNIT_PREFILTER_MAP 72
#define UNIT_BRDF_LUT 73

#define UNIT_VIEWPORT 74

#define UNIT_SHADOW_MAP_FIRST 75
#define UNIT_SHADOW_MAP_LAST (UNIT_SHADOW_MAP_FIRST + MAX_SHADOW_MAP_COUNT - 1)

#define ATTR_POS 0
#define ATTR_NORMAL 1
#define ATTR_TANGENT 2
#define ATTR_TEX 3
#define ATTR_FONT_TEX 4

#define DRAW_TYPE_NONE 0
#define DRAW_TYPE_ELEMENTS 1
#define DRAW_TYPE_ARRAYS 2

#define ENTITY_BILLBOARD_BIT 1
#define ENTITY_SPRITE_BIT 2
#define ENTITY_NO_FRUSTUM_BIT 8

#define STENCIL_MODE_MASK 1
#define STENCIL_MODE_HIGHLIGHT 2

//#define SET_FLOAT_PRECISION
#define SET_FLOAT_PRECISION precision mediump float;

#define MIN_CLAMP_COL_VALUE 0.0
#define MAX_CLAMP_COL_VALUE 10000.0
#define clamp_color(color)\
  color.r = clamp(color.r, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);\
  color.g = clamp(color.g, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);\
  color.b = clamp(color.b, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);

#define LAYOUT_G_BUFFER_OUT\
 layout (location = 0) out vec4 o_fragColor;\
 layout (location = 1) out vec3 o_fragEmission;\
 layout (location = 2) out vec3 o_fragNormal;\
 layout (location = 3) out vec4 o_fragMetal;
// layout (location = 1) out vec4 o_fragSpecular;\
// layout (location = 3) out vec3 o_fragPosition;\

#define LAYOUT_G_BUFFER_SAMPLERS\
 layout(binding = UNIT_G_ALBEDO) uniform sampler2D g_albedo;\
 layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;\
 layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;\
 layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;\
 layout(binding = UNIT_G_METAL) uniform sampler2D g_metal;
// layout(binding = UNIT_G_SPECULAR) uniform sampler2D g_specular;\
// layout(binding = UNIT_G_POSITION) uniform sampler2D g_position;\

#define LAYOUT_OIT_OUT\
 layout (location = 0) out vec4 o_accum;\
 layout (location = 1) out float o_reveal;

#define LAYOUT_OIT_SAMPLERS\
 layout(binding = UNIT_OIT_ACCUMULATOR) uniform sampler2D oit_accumulator;\
 layout(binding = UNIT_OIT_REVEAL) uniform sampler2D oit_reveal;

#define LAYOUT_EFFECT_SAMPLERS\
 layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D effect_albedo;\
 layout(binding = UNIT_EFFECT_BRIGHT) uniform sampler2D effect_bright; \
 layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_work;
