#version 450 core

#include constants.glsl

layout (location = 0) in vec4 a_pos;
#ifdef USE_ALPHA
layout (location = 4) in uint a_materialIndex;
layout (location = 5) in vec2 a_texCoord;
#endif
layout (location = 6) in mat4 a_modelMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} vs_out;
#endif

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

#include fn_calculate_clipping.glsl

void main() {
  vec4 worldPos = a_modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoord = a_texCoord;
#endif

  calculateClipping(worldPos);
}
