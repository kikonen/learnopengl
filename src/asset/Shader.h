#pragma once

const std::string SHADER_TEXTURE{ "tex" };
const std::string SHADER_SPRITE{ "sprite" };

const std::string SHADER_SELECTION{ "selection" };
const std::string SHADER_SELECTION_SPRITE{ "selection_sprite" };

//const std::string SHADER_TERRAIN{ "terrain" };
//const std::string SHADER_WATER{ "water" };
const std::string SHADER_PARTICLE{ "particle" };
const std::string SHADER_NORMAL{ "normal" };
const std::string SHADER_OBJECT_ID{ "object_id" };
const std::string SHADER_OBJECT_ID_SPRITE{ "object_id_sprite" };
//const std::string SHADER_LIGHT{ "light" };
const std::string SHADER_SIMPLE_DEPTH{ "simple_depth" };
const std::string SHADER_DEBUG_DEPTH{ "debug_depth" };
const std::string SHADER_EFFECT{ "effect" };
const std::string SHADER_VIEWPORT{ "viewport" };
const std::string SHADER_VOLUME{ "volume" };
const std::string SHADER_SKYBOX{ "skybox" };

const std::string SHADER_GEOMETRY_PASS{ "g_geometry_pass" };
const std::string SHADER_DEFERRED_PASS{ "g_deferred_pass" };

const std::string CS_FRUSTUM_CULLING{ "frustum_culling" };

const std::string GS_SPRITE{ "sprite" };

const std::string DEF_USE_ALPHA{ "USE_ALPHA" };
const std::string DEF_USE_BLEND{ "USE_BLEND" };
const std::string DEF_USE_NORMAL_TEX{ "USE_NORMAL_TEX" };

const std::string DEF_MAT_COUNT{ "MAT_COUNT" };
const std::string DEF_TEX_COUNT{ "TEX_COUNT" };
const std::string DEF_LIGHT_COUNT{ "LIGHT_COUNT" };
const std::string DEF_CLIP_COUNT{ "CLIP_COUNT" };

const std::string DEF_EFFECT_SUN{ "EFFECT_SUN" };
const std::string DEF_EFFECT_PLASMA{ "EFFECT_PLASMA" };

constexpr int ATTR_POS = 0;
constexpr int ATTR_NORMAL = 1;
constexpr int ATTR_TANGENT = 2;
//constexpr int ATTR_BITANGENT = 3;
constexpr int ATTR_TEX = 4;

//constexpr int ATTR_INSTANCE_ENTITY_INDEX = 5;


// https://www.reddit.com/r/opengl/comments/lz72tk/understanding_dsa_functions_and_buffer_binding/
// https://www.khronos.org/opengl/wiki/Vertex_Specification
constexpr int VBO_VERTEX_BINDING = 0;
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

constexpr int UNIT_G_ALBEDO = 50;
constexpr int UNIT_G_SPECULAR = 51;
constexpr int UNIT_G_EMISSION = 52;
constexpr int UNIT_G_AMBIENT = 53;
constexpr int UNIT_G_POSITION = 54;
constexpr int UNIT_G_NORMAL = 55;

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glActiveTexture.xhtml
constexpr int UNIT_WATER_NOISE = 64;
constexpr int UNIT_WATER_REFLECTION = 65;
constexpr int UNIT_WATER_REFRACTION = 66;
constexpr int UNIT_MIRROR_REFLECTION = 67;
//constexpr int UNIT_MIRROR_RERACTION = 68;
constexpr int UNIT_CUBE_MAP = 69;
constexpr int UNIT_SHADOW_MAP = 70;
constexpr int UNIT_SKYBOX = 71;
constexpr int UNIT_VIEWPORT = 72;

constexpr unsigned int TEXTURE_UNIT_COUNT = 64;
constexpr unsigned int FIRST_TEXTURE_UNIT = 0;
constexpr unsigned int LAST_TEXTURE_UNIT = FIRST_TEXTURE_UNIT + TEXTURE_UNIT_COUNT - 1;

#define ASSERT_TEX_INDEX(texIndex) assert(texIndex >= 0 && texIndex < TEXTURE_COUNT)

#define ASSERT_TEX_UNIT(unitIndex) assert(unitIndex >= FIRST_TEXTURE_UNIT && unitIndex <= LAST_TEXTURE_UNIT)
