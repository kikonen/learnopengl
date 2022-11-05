#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = 4) in uint a_materialIndex;
layout (location = 5) in vec2 a_texCoords;
#endif
layout (location = 6) in mat4 a_modelMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoords;
  flat uint materialIndex;
} vs_out;
#endif

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  vec4 worldPos = a_modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoords = a_texCoords;
#endif

  calculateClipping(worldPos);
}
