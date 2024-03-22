#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_shape.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl
#include ssbo_shapes.glsl

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
  vec3 tangent;
#endif
#ifdef USE_PARALLAX
  vec3 viewTangentPos;
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

#include fn_calculate_normal_pattern.glsl
#include fn_calculate_parallax_mapping.glsl
#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
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

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormalVec2(normal, fs_in.viewPos);
}
