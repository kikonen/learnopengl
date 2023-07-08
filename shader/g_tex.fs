#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_normal_pattern.glsl

void main() {
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.01)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

//  if (material.pattern == 1) {
//    normal = calculateNormalPattern(fs_in.vertexPos, normal);
//  }

//  if (!gl_FrontFacing) {
//    normal = -normal;
//  }

  #include var_calculate_cubemap_diffuse.glsl

  vec4 texColor = material.diffuse;

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  o_fragColor = vec4(texColor.xyz, material.ambient);
  o_fragSpecular = material.specular;
  o_fragEmission = material.emission;

  o_fragPosition = fs_in.worldPos;
  o_fragNormal = normal;
}
