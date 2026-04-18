#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
#ifdef USE_TBN_FS_RECONSTRUCT
  vec4 tangent;
#else
  mat3 tbn;
#endif
#endif
#if defined(USE_PARALLAX) && !defined(USE_TBN_FS_RECONSTRUCT)
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include "include/fn_calculate_parallax_mapping.glsl"
#endif
#include "include/fn_gbuffer_normal_encode.glsl"
#include "include/fn_shape_font.glsl"

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;

  // NOTE KI interpolation from vs to fs denormalizes normal
  vec3 normal = normalize(fs_in.normal);

#ifdef USE_TBN_FS_RECONSTRUCT
  #include "include/var_calculate_tbn.glsl"
  #include "include/apply_parallax_local.glsl"
#else
  #include "include/apply_parallax.glsl"
#endif

  #include "include/var_tex_material.glsl"

#ifdef USE_TBN_FS_RECONSTRUCT
  #include "include/apply_normal_map_local.glsl"
#else
  #include "include/apply_normal_map.glsl"
#endif

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  {
    const vec3 viewDir = -normalize(fs_in.viewPos);
#include "include/var_calculate_cube_map_diffuse.glsl"
  }
#endif

  vec4 color;
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, true, color);

  // NOTE KI alpha/blend does not co-op with line mode
  if (!u_forceLineMode) {
#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (color.a < u_oitMaxBlendThreshold)
    discard;
#else
  if (color.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif
  }

  o_fragColor = color.rgb;
  o_fragMRAS = material.mras;
  o_fragEmission = material.emission;

  #include "include/encode_gbuffer_normal.glsl"
  #include "include/encode_gbuffer_view_position.glsl"
  #include "include/encode_gbuffer_view_z.glsl"
}
