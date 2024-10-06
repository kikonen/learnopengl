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

#ifdef TOY_1
#include fn_shader_toy_main.glsl
#endif
#ifdef TOY_2
#include fn_ray_march_part_1.glsl
#endif

ResolvedMaterial material;

void main()
{
  #include init_shader_toy.glsl

  const uint materialIndex = u_materialIndex;
  const vec2 texCoord = fs_in.texCoord;

  #include var_tex_material.glsl

  vec4 color = material.diffuse;

  vec4 fragColor = vec4(0);
  mainImage(fragColor, gl_FragCoord.xy);
  if (fragColor.a < .0001) {
    // discard;
    color.a = 0.2;
    // color.rgb *= vec4(5, 0.4, 0.4, 1);
#ifdef TOY_1
  color.rgb *= vec3(4, 4, 0.1);
#endif
#ifdef TOY_2
  color.rgb *= vec3(0.4, 8, 0.4);
#endif
  } else {
    color = fragColor;
#ifdef TOY_1
  color.rgb *= vec3(30, 8, 0.4);
#endif
#ifdef TOY_2
  color.rgb *= vec3(0.4, 0.4, 8);
#endif
  color.a = 1;
  }
  // color = vec4(0, 0, 1, 1);
  //color = material.diffuse;

  o_fragColor = color;
}
