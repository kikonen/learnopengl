#version 460 core

layout (location = ATTR_POS) in vec4 a_pos;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

out VS_OUT {
  vec3 fragPos;
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.fragPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
