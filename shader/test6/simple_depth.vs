#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in int aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aModelMatrix;

#include uniform_matrices.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoords;
  flat int materialIndex;
} vs_out;
#endif

void main()
{
  gl_Position = lightSpaceMatrix * aModelMatrix * vec4(aPos, 1.0);

#ifdef USE_ALPHA
  vs_out.materialIndex = aMaterialIndex;
  vs_out.texCoords = aTexCoords;
#endif
}
