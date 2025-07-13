#version 460 core

#include uniform_debug.glsl

in VS_OUT {
  vec3 texCoord;
} fs_in;

layout(early_fragment_tests) in;

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

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
    color = textureLod(u_skybox, fs_in.texCoord, 0);
  }

  o_fragColor = color;
}
