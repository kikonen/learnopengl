#version 460 core

#include constants.glsl

//layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
//layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

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
  Entity entity = u_entities[int(a_entityIndex)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * vec4(pos, 1.0);

  gl_Position = worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.highlightIndex = int(entity.highlightIndex);

  vs_out.scale = vec3(entity.modelMatrix[0][0],
                      entity.modelMatrix[1][1],
                      entity.modelMatrix[2][2]);
}
