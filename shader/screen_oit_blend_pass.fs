#version 460 core

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

#include screen_tri_vertex_out.glsl

LAYOUT_OIT_SAMPLERS;

layout (location = 0) out vec4 o_fragColor;
//layout (location = 1) out vec4 o_fragBright;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;


const float EPSILON = 0.00001f;

float max3(vec3 v)
{
  return max(max(v.x, v.y), v.z);
}

void main()
{
  #include screen_tri_tex_coord.glsl

  float revealage = textureLod(oit_reveal, texCoord, 0).r;

  vec4 accumulation = textureLod(oit_accumulator, texCoord, 0);

  if (isinf(max3(abs(accumulation.rgb))))
    accumulation.rgb = vec3(accumulation.a);

  vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);

  float alpha = clamp(1.0 - revealage, 0.0, 1.0);

  o_fragColor = vec4(averageColor, alpha);

  // const vec3 T = vec3(0.2126, 0.7152, 0.0722);
  // const float brightness = dot(averageColor.xyz, T);

  // if (brightness > 1.0) {
  //   o_fragBright = vec4(averageColor * 0.1, alpha);
  // } else {
  //   o_fragBright = vec4(0.0);
  // }
}
