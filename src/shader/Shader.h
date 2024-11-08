#pragma once

#include <string>

const std::string SHADER_TEXTURE{ "tex" };

const std::string SHADER_SELECTION{ "selection" };

//const std::string SHADER_TERRAIN{ "terrain" };
//const std::string SHADER_WATER{ "water" };
const std::string SHADER_PARTICLE{ "particle" };
const std::string SHADER_SOLID_DECAL{ "g_decal" };
const std::string SHADER_BLEND_DECAL{ "blend_decal" };
const std::string SHADER_NORMAL{ "normal" };
const std::string SHADER_OBJECT_ID{ "object_id" };
//const std::string SHADER_LIGHT{ "light" };
const std::string SHADER_SIMPLE_DEPTH{ "simple_depth" };
const std::string SHADER_DEBUG_DEPTH{ "debug_depth" };
const std::string SHADER_EFFECT{ "effect" };
const std::string SHADER_VIEWPORT{ "viewport" };
const std::string SHADER_VOLUME{ "volume" };
const std::string SHADER_SKYBOX{ "skybox" };
const std::string SHADER_FONT_RENDER{ "font_render" };

const std::string SHADER_HDRI_CUBE_MAP{ "hdri_cube_map" };
const std::string SHADER_IRRADIANCE_CUBE_MAP{ "irradiance_cube_map" };
const std::string SHADER_PREFILTER_CUBE_MAP{ "prefilter_cube_map" };
const std::string SHADER_BRDF_LUT{ "brdf_lut" };

const std::string SHADER_PRE_DEPTH_PASS{ "g_pre_depth" };
const std::string SHADER_GEOMETRY_PASS{ "g_geometry_pass" };
const std::string SHADER_DEFERRED_PASS{ "g_deferred_pass" };
const std::string SHADER_BLOOM_PASS{ "g_bloom_pass" };
const std::string SHADER_BLEND_BLOOM_PASS{ "g_blend_bloom_pass" };
const std::string SHADER_EMISSION_PASS{ "g_emission_pass" };
const std::string SHADER_FOG_PASS{ "g_fog_pass" };
const std::string SHADER_BLEND_OIT_PASS{ "g_blend_oit_pass" };
const std::string SHADER_HDR_GAMMA_PASS{ "g_hdr_gamma_pass" };

const std::string SHADER_OIT_PASS{ "oit_pass" };

const std::string CS_FRUSTUM_CULLING{ "frustum_culling" };

const std::string DEF_USE_ALPHA{ "USE_ALPHA" };
const std::string DEF_USE_BLEND{ "USE_BLEND" };

//const std::string DEF_USE_BILLBOARD{ "USE_BILLBOARD" };

const std::string DEF_USE_DEBUG{ "USE_DEBUG" };

const std::string DEF_USE_PARTICLE{ "USE_PARTICLE" };
const std::string DEF_USE_DECAL{ "USE_DECAL" };

const std::string DEF_USE_TBN{ "USE_TBN" };
const std::string DEF_USE_DUDV_TEX{ "USE_DUDV_TEX" };
const std::string DEF_USE_HEIGHT_TEX{ "USE_HEIGHT_TEX" };
const std::string DEF_USE_DISPLACEMENT_TEX{ "USE_DISPLACEMENT_TEX" };
const std::string DEF_USE_NORMAL_TEX{ "USE_NORMAL_TEX" };
const std::string DEF_USE_PARALLAX{ "USE_PARALLAX" };
const std::string DEF_USE_CUBE_MAP{ "USE_CUBE_MAP" };
const std::string DEF_USE_NORMAL_PATTERN{ "USE_NORMAL_PATTERN" };
const std::string DEF_USE_BONES{ "USE_BONES" };
const std::string DEF_USE_SOCKETS{ "USE_SOCKETS" };

const std::string DEF_MAX_SHADOW_MAP_COUNT{ "MAX_SHADOW_MAP_COUNT" };

const std::string DEF_MAT_COUNT{ "MAT_COUNT" };
const std::string DEF_TEX_COUNT{ "TEX_COUNT" };
const std::string DEF_LIGHT_COUNT{ "LIGHT_COUNT" };
const std::string DEF_CLIP_COUNT{ "CLIP_COUNT" };

const std::string DEF_EFFECT_SUN{ "EFFECT_SUN" };
const std::string DEF_EFFECT_PLASMA{ "EFFECT_PLASMA" };

const std::string DEF_FRUSTUM_DEBUG{ "FRUSTUM_DEBUG" };

const std::string DEF_CS_GROUP_X{ "CS_GROUP_X" };
const std::string DEF_CS_GROUP_Y{ "CS_GROUP_Y" };
const std::string DEF_CS_GROUP_Z{ "CS_GROUP_Z" };

constexpr int ATTR_POS = 0;
constexpr int ATTR_NORMAL = 1;
constexpr int ATTR_TANGENT = 2;
constexpr int ATTR_TEX = 3;
constexpr int ATTR_FONT_ATLAS_TEX = 4;
constexpr int ATTR_BONE_INDEX = 5;
constexpr int ATTR_BONE_WEIGHT = 6;

//constexpr int ATTR_INSTANCE_ENTITY_INDEX = 5;

// https://www.reddit.com/r/opengl/comments/lz72tk/understanding_dsa_functions_and_buffer_binding/
// https://www.khronos.org/opengl/wiki/Vertex_Specification
constexpr int VBO_VERTEX_BINDING = 0;
constexpr int VBO_POSITION_BINDING = 1;
constexpr int VBO_NORMAL_BINDING = 2;
constexpr int VBO_TEXTURE_BINDING = 3;
constexpr int VBO_FONT_ATLAS_BINDING = 4;
constexpr int VBO_BONE_BINDING = 5;
//constexpr int VBO_MODEL_MATRIX_BINDING = 1;
//constexpr int VBO_NORMAL_MATRIX_BINDING = 2;
//constexpr int VBO_OBJECT_ID_BINDING = 3;
//constexpr int VBO_BATCH_BINDING = 1;
//constexpr int VBO_MATERIAL_BINDING = 4;

//constexpr int ATTR_INSTANCE_MODEL_MATRIX_1 = 6;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_2 = 7;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_3 = 8;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_4 = 9;
//
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_1 = 10;
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_2 = 11;
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_3 = 12;
//
//constexpr int ATTR_INSTANCE_OBJECT_ID = 13;
//constexpr int ATTR_INSTANCE_HIGHLIGHT_INDEX = 14;
//constexpr int ATTR_INSTANCE_ENTITY_INDEX = 15;
//constexpr int ATTR_INSTANCE_MATERIAL_INDEX = ATTR_MATERIAL_INDEX;

constexpr int UNIT_CHANNEL_0 = 40;
constexpr int UNIT_CHANNEL_1 = 41;
constexpr int UNIT_CHANNEL_2 = 42;
constexpr int UNIT_CHANNEL_3 = 43;
constexpr int UNIT_CHANNEL_4 = 44;
constexpr int UNIT_CHANNEL_5 = 45;

constexpr int UNIT_FONT_ATLAS = 48;

constexpr int UNIT_HDR_TEXTURE = 49;

constexpr int UNIT_G_ALBEDO = 50;
//constexpr int UNIT_G_SPECULAR = 51;
constexpr int UNIT_G_EMISSION = 52;
//constexpr int UNIT_G_POSITION = 53;
constexpr int UNIT_G_METAL = 53;
constexpr int UNIT_G_NORMAL = 54;
constexpr int UNIT_G_DEPTH = 55;

constexpr int UNIT_OIT_ACCUMULATOR = 56;
constexpr int UNIT_OIT_REVEAL = 57;

constexpr int UNIT_EFFECT_ALBEDO = 58;
constexpr int UNIT_EFFECT_BRIGHT = 59;
constexpr int UNIT_EFFECT_WORK = 60;

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glActiveTexture.xhtml
constexpr int UNIT_WATER_NOISE = 64;
constexpr int UNIT_WATER_REFLECTION = 65;
constexpr int UNIT_WATER_REFRACTION = 66;
constexpr int UNIT_MIRROR_REFLECTION = 67;
constexpr int UNIT_CUBE_MAP = 68;
constexpr int UNIT_SKYBOX = 69;

constexpr int UNIT_ENVIRONMENT_MAP = 70;
constexpr int UNIT_IRRADIANCE_MAP = 71;
constexpr int UNIT_PREFILTER_MAP = 72;
constexpr int UNIT_BDRF_LUT = 73;

constexpr int UNIT_VIEWPORT = 74;

constexpr int MAX_SHADOW_MAP_COUNT_ABS = 4;
constexpr int MAX_SHADOW_MAP_COUNT = 4;
constexpr int UNIT_SHADOW_MAP_FIRST = 75;
constexpr int UNIT_SHADOW_MAP_LAST = UNIT_SHADOW_MAP_FIRST + MAX_SHADOW_MAP_COUNT - 1;

constexpr unsigned int TEXTURE_UNIT_COUNT = 64;
constexpr unsigned int FIRST_TEXTURE_UNIT = 0;
constexpr unsigned int LAST_TEXTURE_UNIT = FIRST_TEXTURE_UNIT + TEXTURE_UNIT_COUNT - 1;

constexpr unsigned int STENCIL_FOG = 1 << 7;
constexpr unsigned int STENCIL_SKYBOX = 1 << 6;
constexpr unsigned int STENCIL_HIGHLIGHT = 1;
constexpr unsigned int STENCIL_OIT = 2;
constexpr unsigned int STENCIL_SOLID = 3;

#define ASSERT_TEX_INDEX(texIndex) assert(texIndex >= 0 && texIndex < MAX_TEXTURE_COUNT)

#define ASSERT_TEX_UNIT(unitIndex) assert(unitIndex >= FIRST_TEXTURE_UNIT && unitIndex <= LAST_TEXTURE_UNIT)
