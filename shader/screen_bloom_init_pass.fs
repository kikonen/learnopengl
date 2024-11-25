#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D scene_albedo;

layout (location = 0) out vec4 o_fragBright;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  const vec2 texCoord = fs_in.texCoord;

  vec4 color = textureLod(scene_albedo, texCoord, 0);

  const vec3 T = vec3(0.2126, 0.7152, 0.0722);
  const float brightness = dot(color.rgb, T);

  if (brightness > 3.0) {
    o_fragBright = vec4(color.rgb, 1.0);
  } else {
    o_fragBright = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
