#version 460 core

#include constants.glsl

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
  Entity entity = u_entities[int(gl_BaseInstance)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.fragPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
