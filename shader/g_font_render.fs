#version 460 core

#include "include/tbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"

#include "include/fn_water_caustics_tbo.glsl"

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
  vec4 tangent;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"

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

  #include "include/var_calculate_tbn.glsl"

  #include "include/apply_parallax.glsl"
  fillMaterialTBO(materialIndex, texCoord);

  #include "include/apply_normal_map_tbo.glsl"

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  {
    const vec3 viewDir = -normalize(fs_in.viewPos);
    fillMaterialReflectTBO(materialIndex);
#include "include/var_calculate_cube_map_diffuse_tbo.glsl"
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

  {
    vec3 worldPos = (u_invViewMatrix * vec4(fs_in.viewPos, 1)).xyz;
    applyWaterCausticTBO(color.rgb, worldPos);
  }

  o_fragColor = color.rgb;
  o_fragMRAS = material.mras;
  o_fragEmission = material.emission;

  #include "include/encode_gbuffer_normal.glsl"
  #include "include/encode_gbuffer_view_position.glsl"
  #include "include/encode_gbuffer_view_z.glsl"
}
