#version 460 core

#include struct_entity.glsl

#include ssbo_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  flat uint materialIndex;
} vs_out;

const vec4 pos = vec4(0.0, -1.0, 0.0, 1.0);
const vec3 normal = vec3(0.0, 0.0, 1.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.u_materialIndex;
  const vec4 worldPos = modelMatrix * pos;

  const vec4 pos = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  gl_PointSize = 64;//(1.0 - pos.z / pos.w) * 64.0;
  gl_Position = pos;
}
