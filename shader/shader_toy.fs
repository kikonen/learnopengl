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

#include fn_shader_toy_main.glsl

ResolvedMaterial material;

void main()
{
  const uint materialIndex = u_materialIndex;
  const vec2 texCoord = fs_in.texCoord;

  #include var_tex_material.glsl

  vec4 color = material.diffuse;

  vec4 fragColor = vec4(0);
  mainImage(fragColor, u_bufferResolution, u_time, gl_FragCoord.xy);
  if (fragColor.a < 0.9) {
    discard;
  } else {
    color = fragColor;
    color.a = 0.9;
  }
  color.a = 0.9;
  // color = vec4(0, 0, 1, 1);

  o_fragColor = color;
}
