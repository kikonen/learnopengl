#include "include/tbo_offsets.glsl"
#include "include/struct_material.glsl"
#include "include/struct_resolved_material.glsl"

void fillMaterialTBO(uint materialIndex, vec2 texCoord)
{
  Material mat;

  // Each Material spans exactly 11 slots of vec4
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  // Slots 0, 1, 2: Raw vector constants
  mat.diffuse  = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_DIFFUSE);
  mat.emission = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_EMISSION);
  mat.mras     = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_MRAS);

  // Slots 3 through 7: Extract Bindless Texture Handles (uvec2 pairs)
  uvec4 slot3 = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_DIFF_EMISS);
  mat.diffuseTex       = slot3.xy;
  mat.emissionTex      = slot3.zw;

  uvec4 slot4 = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_NORM_OPAC);
  mat.normalMapTex     = slot4.xy;
  mat.opacityMapTex    = slot4.zw;

  uvec4 slot5 = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_MRAS_DISP);
  mat.mrasMapTex       = slot5.xy;
#ifdef false
  mat.displacementMapTex = slot5.zw;
#endif

#ifdef false
  uvec4 slot6 = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_DUDV_NOISE);
  mat.dudvMapTex       = slot6.xy;
  mat.noiseMapTex      = slot6.zw;
#endif

#ifdef false
  uvec4 slot7 = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_NOISE2_CUST);
  mat.noise2MapTex     = slot7.xy;
  mat.custom1Tex       = slot7.zw;
#endif

  // Slot 8: Scalar uint flags and physical factors
  uvec4 slot8_uint = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_FLAGS_AND_FACTORS);
  vec4  slot8_float = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_FLAGS_AND_FACTORS);
  mat.flags            = slot8_uint.x;
  mat.reflection       = slot8_float.y;
  mat.refraction       = slot8_float.z;
  mat.refractionRatio  = slot8_float.w;

#ifdef false
  // Slot 9: Tilings and sprite mapping information
  uvec4 slot9_uint  = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TILING_AND_SPRITES);
  mat.spriteCount      = slot9_uint.z;
  mat.spritesX         = slot9_uint.w;
#endif

  // Slot 10: Layer parameters and parallax descriptors
  uvec4 slot10_uint = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_LAYERS_AND_PARALLAX);
  vec4  slot10_float = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_LAYERS_AND_PARALLAX);
  mat.spritesY         = slot10_uint.x;
  mat.layers           = int(slot10_uint.y);
  mat.layersDepth      = slot10_float.z;
  mat.parallaxDepth    = slot10_float.w;

  // ------------------------------------------------=================
  // FIX: Assign scalar values to global 'material' BEFORE resolving textures
  // ----------------------------------------------------------------=
  material.flags         = mat.flags;
  material.layersDepth   = mat.layersDepth;
  material.parallaxDepth = mat.parallaxDepth;

  // ... fill ResolvedMaterial material
  {
    vec4 mrasTex = vec4(0, 1, 1, 0);

    if (mat.mrasMapTex.x > 0) {
      mrasTex = texture(sampler2D(mat.mrasMapTex), texCoord).rgba;

      // Safe now! material.flags contains valid bitfields
      if ((material.flags & MATERIAL_INVERT_METALNESS) != 0) {
        MRA_TEX_METALNESS = 1.0 - MRA_TEX_METALNESS;
      }
      if ((material.flags & MATERIAL_INVERT_ROUGHNESS) != 0) {
        MRA_TEX_ROUGHNESS = 1.0 - MRA_TEX_ROUGHNESS;
      }
      if ((material.flags & MATERIAL_INVERT_OCCLUSION) != 0) {
        MRA_TEX_OCCLUSION = 1.0 - MRA_TEX_OCCLUSION;
      }
    }

    vec4 mras = mat.mras.rgba * mrasTex.rgba;

    material.diffuse = mat.diffuse * texture(sampler2D(mat.diffuseTex), texCoord);

    material.diffuse.a *= texture(sampler2D(mat.opacityMapTex), texCoord).r;

    vec4 emission = texture(sampler2D(mat.emissionTex), texCoord);

    material.emission = mat.emission.rgb * emission.rgb * emission.a;

    material.mras = mras.rgba;
  }
}

void fillMaterialPlainTBO(uint materialIndex)
{
  Material mat;

  // Each Material spans exactly 11 slots of vec4
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  // Slots 0, 1, 2: Raw vector constants
  mat.diffuse  = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_DIFFUSE);
  mat.emission = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_EMISSION);
  mat.mras     = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_MRAS);

  // Slot 8: Scalar uint flags and physical factors
  uvec4 slot8_uint = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_FLAGS_AND_FACTORS);
  vec4  slot8_float = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_FLAGS_AND_FACTORS);
  mat.flags            = slot8_uint.x;
  mat.reflection       = slot8_float.y;
  mat.refraction       = slot8_float.z;
  mat.refractionRatio  = slot8_float.w;

  // ------------------------------------------------=================
  // FIX: Assign scalar values to global 'material' BEFORE resolving textures
  // ----------------------------------------------------------------=
  material.flags         = mat.flags;

  {
    material.diffuse = mat.diffuse;
    material.emission = mat.emission.rgb;
    material.mras = mat.mras.rgba;
  }
}

void fillMaterialParallaxTBO(uint materialIndex)
{
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  float parallaxDepth = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_LAYERS_AND_PARALLAX).w;

  material.parallaxDepth = parallaxDepth;
}

uvec2 readMaterialDisplacementTexTBO(uint materialIndex)
{
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  uvec2 displacementTex = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_MRAS_DISP).zw;

  return displacementTex;
}

uvec2 readMaterialNormalTexTBO(uint materialIndex)
{
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  uvec2 normalTex = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_NORM_OPAC).xy;

  return normalTex;
}

uvec2 readMaterialDudvTexTBO(uint materialIndex)
{
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  uvec2 dudvTex = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_DUDV_NOISE).xy;

  return dudvTex;
}

void fillMaterialTilingTBO(uint materialIndex)
{
  // Calculate the vec4 slot offset
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  // Optimized: Fetches 16 bytes into cache, but maps ONLY 2 registers for the ALU
  vec2 slot9_tiling = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_TILING_AND_SPRITES).xy;

  // Direct register assignment
  material.tilingX = slot9_tiling.x;
  material.tilingY = slot9_tiling.y;
}

// Stripped Down: Resolves alpha values with corrected channel masks
float readMaterialAlphaTBO(uint materialIndex, vec2 texCoord)
{
  const int matBase = int(materialIndex) * MATERIAL_STRIDE_VEC4;

  // 1. Get base material vector alpha (.w is .a)
  float baseAlpha = texelFetch(u_materialFloatTBO, matBase + MAT_SLOT_DIFFUSE).w;

  // 2. Fetch texture handles
  uvec2 diffuseTex  = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_DIFF_EMISS).xy;
  uvec2 opacityTex  = texelFetch(u_materialUintTBO, matBase + MAT_SLOT_TEX_NORM_OPAC).zw;

  // 3. Sample handles with exact color-channel matching
  float diffuseAlpha = texture(sampler2D(diffuseTex), texCoord).a;

  // FIX: Fulfills your original G-Buffer logic by reading the .r channel for opacity mapping
  float opacityMask  = texture(sampler2D(opacityTex), texCoord).r;

  return baseAlpha * diffuseAlpha * opacityMask;
}
