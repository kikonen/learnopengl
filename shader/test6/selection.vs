#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint highlightIndex;
} vs_out;
#else
out VS_OUT {
  flat uint highlightIndex;
} vs_out;
#endif

out float gl_ClipDistance[CLIP_COUNT];

mat4 HIGHLIGHT_MAT = mat4(1.02, 0, 0, 0,
                          0, 1.02, 0, 0,
                          0, 0, 1.02, 0,
                          0, 0, 0, 1);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  const vec4 worldPos = entity.modelMatrix * HIGHLIGHT_MAT * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  const int materialIndex = entity.materialIndex;
  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;
#endif

  vs_out.highlightIndex = entity.highlightIndex;

  calculateClipping(worldPos);
}
