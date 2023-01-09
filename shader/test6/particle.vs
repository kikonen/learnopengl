#version 460 core

#include constants.glsl

#include struct_entity.glsl

#include uniform_entities.glsl
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

void main() {
  Entity entity = u_entities[int(gl_BaseInstance)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * pos;

  vec4 pos = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  gl_PointSize = 64;//(1.0 - pos.z / pos.w) * 64.0;
  gl_Position = pos;
}
