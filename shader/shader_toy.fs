#version 460 core

#include struct_lights.glsl

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
#include uniform_lights.glsl

#include ssbo_materials.glsl

// NOTE KI depth is *not* used
// => for *stencil test
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;


layout(location = UNIFORM_MATERIAL_INDEX) uniform uint u_materialIndex;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include var_shader_toy.glsl

#ifdef SHADER_1
#include shader_toy_main.glsl
#endif
#ifdef SHADER_2
#include shader_toy_ray_march_part_1.glsl
//#include shader_toy_cloud_2.glsl
//#include shader_toy_sdf_shapes.glsl
#endif

ResolvedMaterial material;

void main()
{
  const uint materialIndex = u_materialIndex;
  const vec2 texCoord = fs_in.texCoord;

  #include var_tex_material.glsl
  #include init_shader_toy.glsl

  vec4 color = material.diffuse;

  vec4 fragColor = vec4(0);
  mainImage(fragColor, gl_FragCoord.xy);
  color = fragColor;

// NOTE KI USE_BLEND does not make much sense here
// => written into framebuffer, blend is property of material usign this texture
#ifdef USE_ALPHA
  if (fragColor.a < ALPHA_THRESHOLD)
    discard;
#endif

  // color = vec4(0, 0, 1, 1);
  //color = material.diffuse;

  o_fragColor = color;
}
