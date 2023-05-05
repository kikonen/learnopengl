#version 460 core

in VS_OUT {
  vec3 texCoord;
} fs_in;

layout(early_fragment_tests) in;

#include uniform_data.glsl

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out uint o_fragMaterial;
layout (location = 2) out vec3 o_fragTexCoord;
layout (location = 3) out vec3 o_fragPosition;
layout (location = 4) out vec3 o_fragNormal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

void main() {
  o_fragMaterial = MATERIAL_SKYBOX;
  o_fragTexCoord = fs_in.texCoord;

  o_fragPosition = u_viewWorldPos + 1000 * u_viewFront;
  o_fragNormal = u_viewFront;
}
