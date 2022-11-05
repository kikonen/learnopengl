#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = 4) in uint a_materialIndex;
layout (location = 5) in vec2 a_texCoords;
#endif
layout (location = 6) in mat4 a_modelMatrix;

#include uniform_matrices.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoords;
  flat uint materialIndex;
} vs_out;
#endif

void main()
{
  vec4 worldPos = a_modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_lightSpaceMatrix * worldPos;

#ifdef USE_ALPHA
  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoords = a_texCoords;
#endif
}
