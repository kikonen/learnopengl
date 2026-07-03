#version 460 core

#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"

in VS_OUT {
  vec3 texCoord;
} fs_in;

layout(early_fragment_tests) in;

layout(binding = UNIT_SKYBOX_DAY) uniform samplerCube u_skyboxDay;
#ifdef USE_SKYBOX_NIGHT
layout(binding = UNIT_SKYBOX_NIGHT) uniform samplerCube u_skyboxNight;

// 0 = full day .. 1 = full night; driven by the World day-night model (set in PassSkybox).
// Kept consistent with the sun light by sharing World::skyBlend() (sun elevation + twilight).
uniform float u_skyBlend;
#endif

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  vec4 color;

  if (Debug.u_skyboxColorEnabled) {
    color = vec4(Debug.u_skyboxColor.rgb, 1.0);
  } else {
#ifdef USE_SKYBOX_NIGHT
    vec4 dayColor = textureLod(u_skyboxDay, fs_in.texCoord, 0);
    vec4 nightColor = textureLod(u_skyboxNight, fs_in.texCoord, 0);
    color = mix(dayColor, nightColor, clamp(u_skyBlend, 0.0, 1.0));
#else
    color = textureLod(u_skyboxDay, fs_in.texCoord, 0);
#endif
  }

  o_fragColor = color;
}
