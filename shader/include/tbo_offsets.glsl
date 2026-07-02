// "include/layout_tbo_offsets.glsl"

// ============================================================================
// INSTANCE LAYOUT (80 Bytes = 5 vec4 Chunks)
// ============================================================================
#define INSTANCE_STRIDE_VEC4         5

#define INST_SLOT_TRANSFORM_ROW0     0
#define INST_SLOT_TRANSFORM_ROW1     1
#define INST_SLOT_TRANSFORM_ROW2     2
#define INST_SLOT_INDICES_AND_FLAGS  3 // x: entityIndex, y: materialIndex, z: boneBaseIndex, w: flags
#define INST_SLOT_CASE_DATA          4 // x: case-specific data, yzw: padding

// ============================================================================
// ENTITY LAYOUT (160 Bytes = 10 vec4 Chunks)
// ============================================================================
#define ENTITY_STRIDE_VEC4           10

#define ENT_SLOT_MODEL_ROW0          0
#define ENT_SLOT_MODEL_ROW1          1
#define ENT_SLOT_MODEL_ROW2          2
#define ENT_SLOT_NORMAL_ROW0         3
#define ENT_SLOT_NORMAL_ROW1         4
#define ENT_SLOT_NORMAL_ROW2         5
#define ENT_SLOT_WORLD_VOLUME        6
#define ENT_SLOT_WORLD_SCALE         7
#define ENT_SLOT_OBJECT_AND_FLAGS    8 // xy: fontHandle, z: objectID, w: flags
#define ENT_SLOT_TILING              9 // x: tilingX, y: tilingY, zw: padding

// ============================================================================
// MATERIAL LAYOUT (176 Bytes = 11 vec4 Chunks)
// ============================================================================
#define MATERIAL_STRIDE_VEC4         11

#define MAT_SLOT_DIFFUSE             0
#define MAT_SLOT_EMISSION            1
#define MAT_SLOT_MRAS                2
#define MAT_SLOT_TEX_DIFF_EMISS      3 // xy: diffuseTex, zw: emissionTex
#define MAT_SLOT_TEX_NORM_OPAC       4 // xy: normalMapTex, zw: opacityMapTex
#define MAT_SLOT_TEX_MRAS_DISP       5 // xy: mrasMapTex, zw: displacementMapTex
#define MAT_SLOT_TEX_DUDV_NOISE      6 // xy: dudvMapTex, zw: noiseMapTex
#define MAT_SLOT_TEX_NOISE2_CUST     7 // xy: noise2MapTex, zw: custom1Tex
#define MAT_SLOT_FLAGS_AND_FACTORS   8 // x: flags, y: reflection, z: refraction, w: refractionRatio
#define MAT_SLOT_TILING_AND_SPRITES  9 // x: tilingX, y: tilingY, z: spriteCount, w: spritesX
#define MAT_SLOT_LAYERS_AND_PARALLAX 10// x: spritesY, y: layers, z: layersDepth, w: parallaxDepth
