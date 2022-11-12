#version 450 core

#include constants.glsl

layout (location = 0) in vec4 a_pos;
#ifdef USE_ALPHA
layout (location = 4) in uint a_materialIndex;
layout (location = 5) in vec2 a_texCoord;
#endif
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 13) in vec4 a_objectID;

#include uniform_matrices.glsl

#ifdef USE_ALPHA
out VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} vs_out;
#else
out VS_OUT {
  flat vec4 objectID;
} vs_out;
#endif


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.objectID = a_objectID;

#ifdef USE_ALPHA
  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoord = a_texCoord;
#endif
}
