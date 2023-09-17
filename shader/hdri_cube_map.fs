#version 460 core

in VS_OUT {
  vec3 worldPos;
} fs_in;

// equirectangularMap
layout(binding = UNIT_HDR_TEXTURE) uniform sampler2D u_hdriTexture;

out vec4 u_fragColor;

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
  // make sure to normalize localPos
  vec2 uv = sampleSphericalMap(normalize(fs_in.worldPos));
  vec3 color = texture(u_hdriTexture, uv).rgb;

  color.r = clamp(color.r, MIN_VALUE, MAX_VALUE);
  color.g = clamp(color.g, MIN_VALUE, MAX_VALUE);
  color.b = clamp(color.b, MIN_VALUE, MAX_VALUE);

  u_fragColor = vec4(color, 1.0);
}
