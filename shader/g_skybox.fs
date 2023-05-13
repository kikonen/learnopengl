#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

#include uniform_data.glsl

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec3 o_fragPosition;
layout (location = 4) out vec3 o_fragNormal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

void main() {
  vec4 texColor = texture(u_skybox, texCoord);
  //texColor = vec4(1.0, 0, 0, 1.0);
  //texColor.a = 0.0;

  o_fragColor = vec4(texColor.xyz, 1.0);
  o_fragSpecular = vec4(texColor.xyz, 0);
  o_fragEmission = texColor;

  o_fragPosition = u_viewWorldPos + 1000 * u_viewFront;
  o_fragNormal = u_viewFront;
}
