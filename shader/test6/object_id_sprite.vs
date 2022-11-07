#version 450 core

#include constants.glsl

#ifdef USE_ALPHA
layout (location = 4) in uint a_materialIndex;
#endif
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 13) in vec4 a_objectID;

#include uniform_matrices.glsl
#include uniform_data.glsl

#ifdef USE_ALPHA
out VS_OUT {
  flat vec4 objectID;

  vec3 scale;
  flat uint materialIndex;
} vs_out;

#else
out VS_OUT {
  flat vec4 objectID;

  vec3 scale;
} vs_out;
#endif

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec3 pos = vec3(0.0, -1.0, 0.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * vec4(pos, 1.0);

  gl_Position =  worldPos;

  vs_out.objectID = a_objectID;

#ifdef USE_ALPHA
  vs_out.materialIndex = a_materialIndex;
#endif

  vs_out.scale = vec3(a_modelMatrix[0][0],
                      a_modelMatrix[1][1],
                      a_modelMatrix[2][2]);
}
