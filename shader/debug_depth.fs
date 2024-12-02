#version 460 core

in vec2 texCoord;

layout(location = UNIFORM_SHADOW_NEAR_PLANE) uniform float u_shadowNearPlane;
layout(location = UNIFORM_SHADOW_FAR_PLANE) uniform float u_shadowFarPlane;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC
  const float near = u_shadowNearPlane;
  const float far = u_shadowFarPlane;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
  float depthValue = textureLod(u_viewportTex, texCoord, 0).r;
//  o_fragColor = vec4(vec3(LinearizeDepth(depthValue) / u_farPlane), 1.0); // perspective
  o_fragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
