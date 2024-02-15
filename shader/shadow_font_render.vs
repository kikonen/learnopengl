#version 460 core

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include uniform_matrices.glsl

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_FONT_TEX) in vec2 a_atlasCoord;

out VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
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

  vs_out.atlasCoord = a_atlasCoord;
  vs_out.atlasHandle = entity.u_fontHandle;

  gl_Position = u_projectedMatrix * modelMatrix * vec4(a_pos, 1.0);
}
