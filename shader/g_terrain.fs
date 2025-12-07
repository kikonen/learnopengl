#version 460 core

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_debug.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_gbuffer_encode.glsl
#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  // NOTE KI interpolation from vs to fs denormalizes normal
  vec3 normal = normalize(fs_in.normal);
  #include apply_normal_map.glsl

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos.xyz - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 texColor = material.diffuse;

  if ((fs_in.tileX + fs_in.tileY) % 2 == 0) {
    texColor *= vec4(3.5, 0.7, 0.7, 1);
  }

  o_fragColor = texColor.rgb;
  o_fragMRAS = material.mras;
  o_fragEmission = material.emission;

  #include encode_gbuffer_normal.glsl
  #include encode_gbuffer_view_position.glsl
  #include encode_gbuffer_view_z.glsl
}
