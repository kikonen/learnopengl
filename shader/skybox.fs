#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  o_fragColor = textureLod(u_skybox, texCoord, 0);
}
