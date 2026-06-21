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

const float DAY_SECONDS = 60 * 60 * 24;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  vec4 color;
  float worldTime = u_worldTime;
  worldTime = u_time * 5000;
  // worldTime = DAY_SECONDS * 2.500001;

  float days = floor(worldTime / DAY_SECONDS);
  float dayPart = (worldTime - days * DAY_SECONDS);
  dayPart = modf(worldTime / DAY_SECONDS, days);

  // midday == 0.5
  dayPart = (dayPart - 0.5) * 2.0;
  if (dayPart < 0) {
    dayPart = -dayPart;
  }

  // float dayPart = 0.95;
  dayPart = clamp(dayPart, 0, 1);

  if (dayPart < 0) {
    dayPart = 1;
  }
  if (dayPart > 1) {
    dayPart = 1;
  }

  if (Debug.u_skyboxColorEnabled) {
    color = vec4(Debug.u_skyboxColor.rgb, 1.0);
  } else {
    vec4 color1 = textureLod(u_skyboxDay, fs_in.texCoord, 0);
    vec4 color2 = textureLod(u_skyboxNight, fs_in.texCoord, 0);
    color = mix(color1, color2, dayPart);
  }

  // color *= sin(u_time * 0.25) * 0.49 + 0.51;
  // color = vec4(0, 1, 0, 1);
  // color = vec4(1, 1, 1, 1);

  o_fragColor = color;
}
