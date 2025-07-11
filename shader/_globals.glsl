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

#define MAX_SHADOW_MAP_COUNT_ABS 4
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
#define UBO_DEBUG 7

#define SSBO_MATERIALS 1
//#define SSBO_TEXTURES 2
#define SSBO_ENTITIES 3
// #define SSBO_MATERIAL_INDECES 4
#define SSBO_DRAW_COMMANDS 5
#define SSBO_DRAW_PARAMETERS 6
#define SSBO_PERFORMANCE_COUNTERS 7
// #define SSBO_SHAPES 8
#define SSBO_INSTANCE_INDECES 9
#define SSBO_PARTICLES 10
#define SSBO_DECALS 11
#define SSBO_TERRAIN_TILES 12
#define SSBO_BONE_TRANSFORMS 13
#define SSBO_SOCKET_TRANSFORMS 14

#define UNIFORM_PROJECTION_MATRIX 1
#define UNIFORM_VIEW_MATRIX 2
#define UNIFORM_SHADOW_NEAR_PLANE 3
#define UNIFORM_SHADOW_FAR_PLANE 4
#define UNIFORM_DRAW_PARAMETERS_INDEX 6
#define UNIFORM_STENCIL_MODE 7
// #define UNIFORM_SHADOW_MAP_INDEX 8
#define UNIFORM_EFFECT_BLOOM_HORIZONTAL 9

#define UNIFORM_HDR_TONE_ENABLED 10
#define UNIFORM_HDR_EXPOSURE_ENABLED 11
#define UNIFORM_GAMMA_CORRECT_ENABLED 12

#define UNIFORM_VIEWPORT_TRANSFORM 15
#define UNIFORM_MODEL_MATRIX 16
#define UNIFORM_MATERIAL_INDEX 17

#define UNIFORM_BLEND_FACTOR 20

#define UNIFORM_VIEWPORT 21

#define SUBROUTINE_EFFECT 0

#define UNIT_0 0

#define UNIT_CHANNEL_0 10
#define UNIT_CHANNEL_1 11
#define UNIT_CHANNEL_2 12
#define UNIT_CHANNEL_3 13
#define UNIT_CHANNEL_4 14
#define UNIT_CHANNEL_5 15

#define UNIT_SOURCE 21
#define UNIT_DESTINATION 22

#define UNIT_HDR_TEXTURE 23

#define UNIT_G_ALBEDO 30
//#define UNIT_G_SPECULAR 31
#define UNIT_G_EMISSION 32
#define UNIT_G_NORMAL 33
#define UNIT_G_MRA 34
#define UNIT_G_VIEW_POSITION 35
#define UNIT_G_VIEW_Z 36
#define UNIT_G_DEPTH 37
#define UNIT_G_DEPTH_COPY 38

#define UNIT_OIT_ACCUMULATOR 40
#define UNIT_OIT_REVEAL 41

#define UNIT_EFFECT_ALBEDO 42
#define UNIT_EFFECT_BRIGHT 43
#define UNIT_EFFECT_WORK 44

#define UNIT_SSAO 46
#define UNIT_SSAO_BLUR 47
#define UNIT_NOISE 48

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
#define ATTR_TEX 1
#define ATTR_NORMAL 2
#define ATTR_TANGENT 3
#define ATTR_FONT_ATLAS_TEX 4
#define ATTR_BONE_INDEX 5
#define ATTR_BONE_WEIGHT 6

#define DRAW_TYPE_NONE 0
#define DRAW_TYPE_ELEMENTS 1
#define DRAW_TYPE_ARRAYS 2

#define ENTITY_NO_FRUSTUM_BIT 4

#define INSTANCE_BILLBOARD_BIT 1

#define STENCIL_MODE_SHIFT_NONE 0
#define STENCIL_MODE_SHIFT_UP 1
#define STENCIL_MODE_SHIFT_LEFT 2
#define STENCIL_MODE_SHIFT_RIGHT 3
#define STENCIL_MODE_SHIFT_DOWN 4

#define BLEND_THRESHOLD 0.95
#define OIT_MAX_BLEND_THRESHOLD 0.995
#define OIT_MIN_BLEND_THRESHOLD 0.001

// TODO KI alpha threshold was adjusted really low (0.01) due to decals seemingly
// => conflicts with other cases (like Fence007 opacity_map)
// => earlier was 0.05 (ccaeb4feea8c49fe48ede73e87987753846ff8a5)
#define ALPHA_THRESHOLD 0.2
#define GBUFFER_ALPHA_THRESHOLD 0.2
#define SHADOW_ALPHA_THRESHOLD 0.35

//#define SET_FLOAT_PRECISION
#define SET_FLOAT_PRECISION precision mediump float;

#define MIN_CLAMP_COL_VALUE 0.0
#define MAX_CLAMP_COL_VALUE 10000.0
#define clamp_color(color)\
  color.r = clamp(color.r, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);\
  color.g = clamp(color.g, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);\
  color.b = clamp(color.b, MIN_CLAMP_COL_VALUE, MAX_CLAMP_COL_VALUE);

#define LAYOUT_G_BUFFER_OUT\
 layout (location = 0) out vec3 o_fragColor;\
 layout (location = 1) out vec3 o_fragEmission;\
 layout (location = 2) out vec3 o_fragNormal;\
 layout (location = 3) out vec3 o_fragMRA;\
 layout (location = 4) out vec3 o_viewPosition;
 // layout (location = 5) out float o_fragViewZ;
// layout (location = 1) out vec4 o_fragSpecular;\
// layout (location = 3) out vec3 o_fragPosition;\

#define LAYOUT_G_BUFFER_SAMPLERS\
 layout(binding = UNIT_G_ALBEDO) uniform sampler2D g_albedo;\
 layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;\
 layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;\
 layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;\
 layout(binding = UNIT_G_MRA) uniform sampler2D g_mra;\
 layout(binding = UNIT_G_VIEW_POSITION) uniform sampler2D g_viewPosition;
 // layout(binding = UNIT_G_VIEW_Z) uniform sampler2D g_viewZ;\
// layout(binding = UNIT_G_SPECULAR) uniform sampler2D g_specular;\

#define LAYOUT_OIT_OUT\
 layout (location = 0) out vec4 o_accum;\
 layout (location = 1) out float o_reveal;\
 layout (location = 2) out vec3 o_fragEmission;

#define LAYOUT_OIT_SAMPLERS\
 layout(binding = UNIT_OIT_ACCUMULATOR) uniform sampler2D oit_accumulator;\
 layout(binding = UNIT_OIT_REVEAL) uniform sampler2D oit_reveal;

#define LAYOUT_EFFECT_SAMPLERS\
 layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D effect_albedo;\
 layout(binding = UNIT_EFFECT_BRIGHT) uniform sampler2D effect_bright; \
 layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_work;
