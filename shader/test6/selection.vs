#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 aModelMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out vec3 fragPos;
out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  calculateClipping(aModelMatrix * vec4(aPos, 1.0));

  fragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
}
