#version 450 core

#include constants.glsl

layout (location = 0) in vec3 aPos;
layout (location = 4) in vec3 aNormal;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

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
  gl_Position = u_projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  calculateClipping(aModelMatrix * vec4(aPos, 1.0));

  vs_out.fragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
  vs_out.normal = normalize(aNormalMatrix * aNormal);
}
