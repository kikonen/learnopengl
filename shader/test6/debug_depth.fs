#version 460 core
out vec4 oFragColor;

in vec2 texCoord;

uniform float u_nearPlane;
uniform float u_farPlane;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC
  return (2.0 * u_nearPlane * u_farPlane) / (u_farPlane + u_nearPlane - z * (u_farPlane - u_nearPlane));
}

void main()
{
  float depthValue = texture(u_viewportTex, texCoord).r;
//  fragColor = vec4(vec3(LinearizeDepth(depthValue) / u_farPlane), 1.0); // perspective
  oFragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
