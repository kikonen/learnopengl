#version 460 core

layout (location = 0) out vec4 o_fragColor;

in vec2 texCoord;

layout(location = UNIFORM_NEAR_PLANE) uniform float u_nearPlane;
layout(location = UNIFORM_FAR_PLANE) uniform float u_farPlane;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC
  return (2.0 * u_nearPlane * u_farPlane) / (u_farPlane + u_nearPlane - z * (u_farPlane - u_nearPlane));
}

void main()
{
  float depthValue = textureLod(u_viewportTex, texCoord, 0).r;
//  o_fragColor = vec4(vec3(LinearizeDepth(depthValue) / u_farPlane), 1.0); // perspective
  o_fragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
