#version 460 core

#include constants.glsl

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  flat vec4 objectID;

  vec3 scale;
  flat uint materialIndex;
} vs_out;

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec4 pos = vec4(0.0, -1.0, 0.0, 1.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Entity entity = u_entities[int(gl_BaseInstance)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * pos;

  gl_Position =  worldPos;

  vs_out.objectID = entity.objectID;

  vs_out.materialIndex = materialIndex;

  vs_out.scale = vec3(entity.modelMatrix[0][0],
                      entity.modelMatrix[1][1],
                      entity.modelMatrix[2][2]);
}
