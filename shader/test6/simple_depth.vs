#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in float aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aModelMatrix;

#include uniform_matrices.glsl

out VS_OUT {
  vec2 texCoords;
  flat int materialIndex;
} vs_out;

void main()
{
  gl_Position = lightSpaceMatrix * aModelMatrix * vec4(aPos, 1.0);
  vs_out.materialIndex = int(aMaterialIndex);
  vs_out.texCoords = aTexCoords;
}
