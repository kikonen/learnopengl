#version 450 core

#include constants.glsl

layout (location = 4) in uint a_materialIndex;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 13) in vec4 a_objectID;

#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  flat vec4 objectID;

  vec3 scale;
  flat uint materialIndex;
} vs_out;

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec3 pos = vec3(0.0, -1.0, 0.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * vec4(pos, 1.0);

  gl_Position =  worldPos;

  vs_out.objectID = a_objectID;

  vs_out.materialIndex = a_materialIndex;

  vs_out.scale = vec3(a_modelMatrix[0][0],
                      a_modelMatrix[1][1],
                      a_modelMatrix[2][2]);
}
