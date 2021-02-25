#version 450 core
out vec4 fragColor;

in vec2 texCoords;

uniform float nearPlane;
uniform float farPlane;
uniform sampler2D viewportTexture;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
  float depthValue = texture(viewportTexture, texCoords).r;
//  fragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0); // perspective
  fragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
