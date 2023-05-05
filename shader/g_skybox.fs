#version 460 core
in vec3 texCoord;

layout(early_fragment_tests) in;

#include uniform_data.glsl

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec4 o_fragAmbient;
layout (location = 4) out vec3 o_fragPosition;
layout (location = 5) out vec3 o_fragNormal;
layout (location = 6) out uint o_fragMaterial;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

void main() {
  vec4 texColor = texture(u_skybox, texCoord);
  //texColor = vec4(1.0, 0, 0, 1.0);
  texColor.a = 0.0;

  o_fragMaterial = MATERIAL_SKYBOX;
  o_fragColor = texColor;
  o_fragSpecular = texColor;
  o_fragSpecular.a = 0;
  o_fragEmission = texColor;
  o_fragAmbient = texColor;

  o_fragPosition = u_viewWorldPos + 1000 * u_viewFront;
  o_fragNormal = u_viewFront;
}
