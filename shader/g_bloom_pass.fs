#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(location = UNIFORM_EFFECT_BLOOM_ITERATION) uniform uint u_effectBloomIteration;

layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_work;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  vec2 offset = 1.0 / textureSize(effect_work, 0);
  vec3 color = texture(effect_work, texCoord).rgb * weight[0];

  if (u_effectBloomIteration % 2 == 0) {
    for (int i = 1; i < 5; ++i) {
      color += texture(effect_work, texCoord + vec2(offset.x * i, 0.0)).rgb * weight[i];
      color += texture(effect_work, texCoord - vec2(offset.x * i, 0.0)).rgb * weight[i];
    }
  } else {
    for (int i = 1; i < 5; ++i) {
      color += texture(effect_work, texCoord + vec2(0.0, offset.y * i)).rgb * weight[i];
      color += texture(effect_work, texCoord - vec2(0.0, offset.y * i)).rgb * weight[i];
    }
  }

  o_fragColor = vec4(color, 1.0);
}
