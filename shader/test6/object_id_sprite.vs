#version 460 core

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

#include fn_convert_object_id.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = modelMatrix * pos;

  gl_Position =  worldPos;

  vs_out.objectID = convertObjectID(entity.objectID);

  vs_out.materialIndex = materialIndex;

  vs_out.scale = vec3(modelMatrix[0][0],
                      modelMatrix[1][1],
                      modelMatrix[2][2]);
}
