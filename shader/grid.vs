#version 460 core

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include texture_plane.glsl
#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl

out VS_OUT {
  vec3 worldPos;
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

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  const uint idx = VERTEX_INDECES[gl_VertexID - gl_BaseVertex];

  vec4 pos = vec4(VERTEX_POS[idx] * entity.u_worldScale.xyz, 1.0);
  pos.x += u_cameraPos.x;
  pos.z += u_cameraPos.z;
  pos.y += 0.0003;

  vec4 worldPos = pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.texCoord = VERTEX_TEX_COORD[idx];
  vs_out.materialIndex = materialIndex;
  vs_out.worldPos = pos.xyz;
}
