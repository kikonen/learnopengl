#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
#ifdef USE_NORMAL_PATTERN
  vec3 vertexPos;
#endif

  flat uint materialIndex;
  flat uint shapeIndex;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeights;
#endif
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_calculate_normal_pattern.glsl
#include fn_calculate_parallax_mapping.glsl
#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < 0.7)
    discard;
#else
  if (material.diffuse.a < 0.05)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

//  if (material.pattern == 1) {
//    normal = calculateNormalPattern(fs_in.vertexPos, normal);
//  }

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 color = material.diffuse;

  clamp_color(color);

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  o_fragColor = color.rgb;
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.rgb;

#ifdef USE_BONES
  // o_fragColor = vec3(1.0, 0.0, 0.0);

  // if (fs_in.boneIndex.x > 150) {
  //   o_fragColor = vec3(0.0, 0.0, 1.0);
  // }
#ifdef USE_DEBUG
  if (u_debugBoneWeight) {
    uint tbi = u_debugBoneIndex;

    uvec4 bi = fs_in.boneIndex;
    vec4 wi = fs_in.boneWeights;

    // tbi = 4;
    // bi = uvec4(4, 0, 0, 0);
    // wi = vec4(1, 0, 0, 0);

    float w = 0;
    if (bi.x == tbi && wi.x > 0) w += 0.25;
    if (bi.y == tbi && wi.y > 0) w += 0.25;
    if (bi.z == tbi && wi.z > 0) w += 0.25;
    if (bi.w == tbi && wi.w > 0) w += 0.25;

    if (w > 0) {
      vec3 shade;
      if (w >= 1.0) {
	shade = vec3(1.0, 0, 0);
      } else if (w >= 0.75) {
	shade = vec3(0, 1.0, 1.0);
      } else if (w >= 0.5) {
	shade = vec3(0, 1.0, 0);
      } else if (w >= 0.25) {
	shade = vec3(0, 0, 1.0);
      } else {
	shade = vec3(1.0, 1.0, 0);
      }

      o_fragColor = shade;
    }
    //o_fragColor = fs_in.boneWeights.rgb;
    // o_fragColor = vec3(1.0, 0, 0);
  }
#endif
#endif

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
