#version 460 core

#include include/struct_entity.glsl

#include include/ssbo_entities.glsl
#include include/uniform_matrices.glsl

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

SET_FLOAT_PRECISION;

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include include/var_entity_model_matrix.glsl
  #include include/var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;
  const vec4 worldPos = modelMatrix * vec4(pos, 1.0);

  gl_Position = worldPos;

  vs_out.materialIndex = materialIndex;

  vs_out.scale = entity.u_worldScale.xyz;
}
