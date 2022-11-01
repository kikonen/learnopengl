#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 4) in vec3 a_normal;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  vec4 worldPos = a_modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;

  calculateClipping(worldPos);

  vs_out.fragPos = worldPos.xyz;
  vs_out.normal = normalize(a_normalMatrix * a_normal);
}
