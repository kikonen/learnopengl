#version 460 core

#include uniform_buffer_info.glsl

// in VS_OUT {
//   vec3 worldPos;
// } fs_in;

in VS_OUT {
  vec2 texCoord;
  vec3 worldPos;
  flat mat4 projected;
} fs_in;

// equirectangularMap
layout(binding = UNIT_HDR_TEXTURE) uniform sampler2D u_hdriTexture;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec2 invAtan = vec2(0.1591, 0.3183);

const float MIN_VALUE = 0.0;
const float MAX_VALUE = 10000.0;

vec2 sampleSphericalMap(vec3 v)
{
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main()
{
const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;

  // make sure to normalize localPos
  vec3 worldPos = vec3(0, texCoord.x, texCoord.y);
  vec2 uv = sampleSphericalMap(normalize(worldPos));
  vec3 color = texture(u_hdriTexture, uv).rgb;

  color.r = clamp(color.r, MIN_VALUE, MAX_VALUE);
  color.g = clamp(color.g, MIN_VALUE, MAX_VALUE);
  color.b = clamp(color.b, MIN_VALUE, MAX_VALUE);

  // color *= vec3(1, 2, 1);
  // color.rg = texCoord;

  color = vec3(fs_in.projected[2][0]);
  color = normalize((fs_in.projected * vec4(1)).rgb);
  color = color * 0.5 + 0.5;
  // color = vec3(color) * vec3(0, 3, 0);

  o_fragColor = vec4(color, 1.0);
  // o_fragColor = vec4(1, 0, 0, 1.0);
}
