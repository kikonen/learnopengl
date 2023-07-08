#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  o_fragColor = texture(u_skybox, texCoord);
  //o_fragColor = vec4(1.0, 0, 0, 1.0);
}
