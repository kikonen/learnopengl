#version 460 core

#include struct_material.glsl

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 worldPos;
  vec2 texCoord;
  vec3 viewPos;

  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

#include fn_calculate_fog.glsl

void main() {
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.01)
    discard;
#endif
#endif

  vec4 texColor = material.diffuse;


#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.01)
    discard;
#endif
#endif

//  texColor.a = clamp(texColor.a, 0, 0.5);

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  texColor = calculateFog(fs_in.viewPos, texColor);

  o_fragColor = texColor;
//  o_fragColor = vec4(1, 0, 0, 1);
}
