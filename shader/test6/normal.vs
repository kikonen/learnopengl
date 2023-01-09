#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

out VS_OUT {
  vec3 normal;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  Entity entity = u_entities[int(gl_BaseInstance)];
  mat3 normalMatrix = mat3(entity.normalMatrix);
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_viewMatrix * worldPos;

  vs_out.normal = normalize(mat3(u_viewMatrix) * normalMatrix * a_normal);
}
