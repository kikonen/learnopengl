#version 460 core

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"

#include "include/ssbo_instance_indeces.glsl"

#include "include/texture_plane.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"

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

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"

void main()
{
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  const uint idx = VERTEX_INDECES[gl_VertexID - gl_BaseVertex];

  vec4 pos = vec4(VERTEX_POS[idx], 1.0);

  vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.texCoord = VERTEX_TEX_COORD[idx];
  vs_out.materialIndex = materialIndex;
}
