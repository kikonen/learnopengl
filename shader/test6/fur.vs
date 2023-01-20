#version 460 core

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = entity.modelMatrix * a_pos;
  const mat3 normalMatrix = transpose(inverse(mat3(u_viewMatrix * entity.modelMatrix)));

  const mat4 vmMat = u_viewMatrix * entity.modelMatrix;

  gl_Position = vmMat * a_pos;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord = a_texCoord * u_materials[materialIndex].tiling;

  vs_out.normal = normalize(normalMatrix * a_normal);

  calculateClipping(worldPos);
}
