#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

#include uniform_data.glsl

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_gbuffer_encode.glsl

void main() {
  vec4 texColor = texture(u_skybox, texCoord);
  //texColor = vec4(1.0, 0, 0, 1.0);
  //texColor.a = 0.0;

  o_fragColor = vec4(texColor.xyz, 1.0);
  o_fragSpecular = vec4(texColor.xyz, 0);
  o_fragEmission = texColor.xyz;

  //o_fragPosition = u_viewWorldPos + 1000 * u_viewFront;
  o_fragNormal = encodeGNormal(u_viewFront);
}
