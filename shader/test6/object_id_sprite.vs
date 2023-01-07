#version 460 core

#include constants.glsl

//layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
//layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

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
  Entity entity = u_entities[int(a_entityIndex)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * pos;

  gl_Position =  worldPos;

  vs_out.objectID = entity.objectID;

  vs_out.materialIndex = materialIndex;

  vs_out.scale = vec3(entity.modelMatrix[0][0],
                      entity.modelMatrix[1][1],
                      entity.modelMatrix[2][2]);
}
