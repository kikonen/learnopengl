#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

LAYOUT_OIT_SAMPLERS;

layout (location = 0) out vec4 o_fragColor;

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
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  float revealage = texture(oit_reveal, texCoord).r;

  vec4 accumulation = texture(oit_accumulator, texCoord, 0);

  if (isinf(max3(abs(accumulation.rgb))))
    accumulation.rgb = vec3(accumulation.a);

  vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);

  o_fragColor = vec4(averageColor, 1.0 - revealage);
}
