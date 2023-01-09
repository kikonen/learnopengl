#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat uint materialIndex;
  vec2 texCoord;
  vec3 fragPos;
  vec3 normal;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  Entity entity = u_entities[int(gl_BaseInstance)];
  mat3 normalMatrix = mat3(entity.normalMatrix);
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

  calculateClipping(worldPos);

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;

  vs_out.fragPos = worldPos.xyz;
  vs_out.normal = normalize(normalMatrix * a_normal);
}
