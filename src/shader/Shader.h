#pragma once

#include <string>

inline const std::string SHADER_TEXTURE{ "tex" };
inline const std::string SHADER_G_TEX{ "g_tex" };

inline const std::string SHADER_SELECTION{ "selection" };
inline const std::string SHADER_NORMAL{ "normal" };
inline const std::string SHADER_OBJECT_ID{ "object_id" };

inline const std::string SHADER_SHADOW{ "shadow" };
inline const std::string SHADER_VIEWPORT{ "viewport" };
inline const std::string SHADER_VOLUME{ "volume" };

inline const std::string SHADER_PRE_DEPTH_PASS{ "g_pre_depth" };

inline const std::string SHADER_OIT_PASS{ "oit_pass" };

inline const std::string DEF_USE_ALPHA{ "USE_ALPHA" };
inline const std::string DEF_USE_BLEND{ "USE_BLEND" };

//inline const std::string DEF_USE_BILLBOARD{ "USE_BILLBOARD" };

inline const std::string DEF_USE_DEBUG{ "USE_DEBUG" };

inline const std::string DEF_USE_PARTICLE{ "USE_PARTICLE" };
inline const std::string DEF_USE_DECAL{ "USE_DECAL" };

inline const std::string DEF_USE_TBN{ "USE_TBN" };
inline const std::string DEF_USE_DUDV_TEX{ "USE_DUDV_TEX" };
inline const std::string DEF_USE_HEIGHT_TEX{ "USE_HEIGHT_TEX" };
inline const std::string DEF_USE_DISPLACEMENT_TEX{ "USE_DISPLACEMENT_TEX" };
inline const std::string DEF_USE_NORMAL_TEX{ "USE_NORMAL_TEX" };
inline const std::string DEF_USE_PARALLAX{ "USE_PARALLAX" };
inline const std::string DEF_USE_CUBE_MAP{ "USE_CUBE_MAP" };
inline const std::string DEF_USE_NORMAL_PATTERN{ "USE_NORMAL_PATTERN" };
inline const std::string DEF_USE_BONES{ "USE_BONES" };
inline const std::string DEF_USE_SOCKETS{ "USE_SOCKETS" };

inline const std::string DEF_MAX_SHADOW_MAP_COUNT{ "MAX_SHADOW_MAP_COUNT" };

inline const std::string DEF_MAT_COUNT{ "MAT_COUNT" };
inline const std::string DEF_TEX_COUNT{ "TEX_COUNT" };
inline const std::string DEF_LIGHT_COUNT{ "LIGHT_COUNT" };
inline const std::string DEF_CLIP_COUNT{ "CLIP_COUNT" };

inline const std::string DEF_EFFECT_SUN{ "EFFECT_SUN" };
inline const std::string DEF_EFFECT_PLASMA{ "EFFECT_PLASMA" };

inline const std::string DEF_FRUSTUM_DEBUG{ "FRUSTUM_DEBUG" };

inline const std::string DEF_CS_GROUP_X{ "CS_GROUP_X" };
inline const std::string DEF_CS_GROUP_Y{ "CS_GROUP_Y" };
inline const std::string DEF_CS_GROUP_Z{ "CS_GROUP_Z" };

inline constexpr int ATTR_POS = 0;
inline constexpr int ATTR_TEX = 1;
inline constexpr int ATTR_NORMAL = 2;
inline constexpr int ATTR_TANGENT = 3;
inline constexpr int ATTR_FONT_ATLAS_TEX = 4;
inline constexpr int ATTR_BONE_INDEX = 5;
inline constexpr int ATTR_BONE_WEIGHT = 6;

//inline constexpr int ATTR_INSTANCE_ENTITY_INDEX = 5;

// https://www.reddit.com/r/opengl/comments/lz72tk/understanding_dsa_functions_and_buffer_binding/
// https://www.khronos.org/opengl/wiki/Vertex_Specification
inline constexpr int VBO_VERTEX_BINDING = 0;
inline constexpr int VBO_POSITION_BINDING = 1;
inline constexpr int VBO_NORMAL_BINDING = 2;
inline constexpr int VBO_TANGENT_BINDING = 3;
inline constexpr int VBO_TEXTURE_BINDING = 4;
inline constexpr int VBO_FONT_ATLAS_BINDING = 5;
inline constexpr int VBO_BONE_BINDING = 6;
//inline constexpr int VBO_MODEL_MATRIX_BINDING = 1;
//inline constexpr int VBO_NORMAL_MATRIX_BINDING = 2;
//inline constexpr int VBO_OBJECT_ID_BINDING = 3;
//inline constexpr int VBO_BATCH_BINDING = 1;
//inline constexpr int VBO_MATERIAL_BINDING = 4;

//inline constexpr int ATTR_INSTANCE_MODEL_MATRIX_1 = 6;
//inline constexpr int ATTR_INSTANCE_MODEL_MATRIX_2 = 7;
//inline constexpr int ATTR_INSTANCE_MODEL_MATRIX_3 = 8;
//inline constexpr int ATTR_INSTANCE_MODEL_MATRIX_4 = 9;
//
//inline constexpr int ATTR_INSTANCE_NORMAL_MATRIX_1 = 10;
//inline constexpr int ATTR_INSTANCE_NORMAL_MATRIX_2 = 11;
//inline constexpr int ATTR_INSTANCE_NORMAL_MATRIX_3 = 12;
//
//inline constexpr int ATTR_INSTANCE_OBJECT_ID = 13;
//inline constexpr int ATTR_INSTANCE_HIGHLIGHT_INDEX = 14;
//inline constexpr int ATTR_INSTANCE_ENTITY_INDEX = 15;
//inline constexpr int ATTR_INSTANCE_MATERIAL_INDEX = ATTR_MATERIAL_INDEX;

inline constexpr int UNIT_0 = 0;

inline constexpr int UNIT_CHANNEL_0 = 10;
inline constexpr int UNIT_CHANNEL_1 = 11;
inline constexpr int UNIT_CHANNEL_2 = 12;
inline constexpr int UNIT_CHANNEL_3 = 13;
inline constexpr int UNIT_CHANNEL_4 = 14;
inline constexpr int UNIT_CHANNEL_5 = 15;

inline constexpr int UNIT_SOURCE = 21;
inline constexpr int UNIT_DESTINATION = 22;

inline constexpr int UNIT_HDR_TEXTURE = 23;

inline constexpr int UNIT_G_ALBEDO = 30;
//inline constexpr int UNIT_G_SPECULAR = 31;
inline constexpr int UNIT_G_EMISSION = 32;
inline constexpr int UNIT_G_NORMAL = 33;
inline constexpr int UNIT_G_MRAO = 34;
inline constexpr int UNIT_G_VIEW_POSITION = 35;
inline constexpr int UNIT_G_VIEW_Z = 36;
inline constexpr int UNIT_G_DEPTH = 37;
inline constexpr int UNIT_G_DEPTH_COPY = 38;

inline constexpr int UNIT_OIT_ACCUMULATOR = 40;
inline constexpr int UNIT_OIT_REVEAL = 41;

inline constexpr int UNIT_EFFECT_ALBEDO = 42;
inline constexpr int UNIT_EFFECT_BRIGHT = 43;
inline constexpr int UNIT_EFFECT_WORK = 44;

inline constexpr int UNIT_SSAO = 46;
inline constexpr int UNIT_SSAO_BLUR = 47;
inline constexpr int UNIT_NOISE = 48;

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glActiveTexture.xhtml
inline constexpr int UNIT_WATER_NOISE = 64;
inline constexpr int UNIT_WATER_REFLECTION = 65;
inline constexpr int UNIT_WATER_REFRACTION = 66;
inline constexpr int UNIT_MIRROR_REFLECTION = 67;
inline constexpr int UNIT_CUBE_MAP = 68;
inline constexpr int UNIT_SKYBOX = 69;

inline constexpr int UNIT_ENVIRONMENT_MAP = 70;
inline constexpr int UNIT_IRRADIANCE_MAP = 71;
inline constexpr int UNIT_PREFILTER_MAP = 72;
inline constexpr int UNIT_BDRF_LUT = 73;

inline constexpr int UNIT_VIEWPORT = 74;

inline constexpr int MAX_SHADOW_MAP_COUNT_ABS = 4;
inline constexpr int MAX_SHADOW_MAP_COUNT = 4;
inline constexpr int UNIT_SHADOW_MAP_FIRST = 75;
inline constexpr int UNIT_SHADOW_MAP_LAST = UNIT_SHADOW_MAP_FIRST + MAX_SHADOW_MAP_COUNT - 1;

inline constexpr unsigned int TEXTURE_UNIT_COUNT = 64;
inline constexpr unsigned int FIRST_TEXTURE_UNIT = 0;
inline constexpr unsigned int LAST_TEXTURE_UNIT = FIRST_TEXTURE_UNIT + TEXTURE_UNIT_COUNT - 1;

inline constexpr unsigned int STENCIL_FOG = 1 << 7;
inline constexpr unsigned int STENCIL_SKYBOX = 1 << 6;
inline constexpr unsigned int STENCIL_HIGHLIGHT = 1;
inline constexpr unsigned int STENCIL_OIT = 2;
inline constexpr unsigned int STENCIL_SOLID = 3;

#define ASSERT_TEX_INDEX(texIndex) assert(texIndex >= 0 && texIndex < MAX_TEXTURE_COUNT)

#define ASSERT_TEX_UNIT(unitIndex) assert(unitIndex >= FIRST_TEXTURE_UNIT && unitIndex <= LAST_TEXTURE_UNIT)
