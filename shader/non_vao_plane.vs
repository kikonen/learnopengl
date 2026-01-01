#version 460 core

#include include/ssbo_entities.glsl
#include include/ssbo_instance_indeces.glsl

#include include/texture_plane.glsl

#include include/uniform_matrices.glsl
#include include/uniform_camera.glsl

out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

void main()
{
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include include/var_entity_model_matrix.glsl
  #include include/var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  const uint idx = VERTEX_INDECES[gl_VertexID - gl_BaseVertex];

  vec4 pos = vec4(VERTEX_POS[idx], 1.0);

  vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.texCoord = VERTEX_TEX_COORD[idx];
  vs_out.materialIndex = materialIndex;
}
