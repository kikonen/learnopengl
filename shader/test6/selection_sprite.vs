#version 460 core

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

out VS_OUT {
  vec3 scale;
  flat uint materialIndex;
  flat uint highlightIndex;
} vs_out;

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec3 pos = vec3(0.0, -1.0, 0.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = entity.modelMatrix * vec4(pos, 1.0);

  gl_Position = worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.highlightIndex = entity.highlightIndex;

  vs_out.scale = vec3(entity.modelMatrix[0][0],
                      entity.modelMatrix[1][1],
                      entity.modelMatrix[2][2]);
}
