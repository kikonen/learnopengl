#version 460 core

#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"

in VS_OUT {
  vec3 texCoord;
} fs_in;

layout(early_fragment_tests) in;

layout(binding = UNIT_SKYBOX_DAY) uniform samplerCube u_skyboxDay;
layout(binding = UNIT_SKYBOX_NIGHT) uniform samplerCube u_skyboxNight;

layout (location = 0) out vec4 o_fragColor;

const uint DAY_SECONDS = 60 * 60 * 24u;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  vec4 color;

  // const int days = floor(u_worldTime / DAY_SECONDS);
  // const float dayPart = u_worldTime - float(days * DAY_SECONDS);
  float dayPart = 1.0;

  if (Debug.u_skyboxColorEnabled) {
    color = vec4(Debug.u_skyboxColor.rgb, 1.0);
  } else {
    vec4 color1 = textureLod(u_skyboxDay, fs_in.texCoord, 0);
    vec4 color2 = textureLod(u_skyboxNight, fs_in.texCoord, 0);
    color = mix(color1, color2, dayPart);
  }

  o_fragColor = color;
}
