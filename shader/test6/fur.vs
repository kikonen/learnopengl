#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 4) in int aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 normal;
  vec2 texCoords;

  flat int materialIndex;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  mat4 vmMat = viewMatrix * aModelMatrix;

  gl_Position = vmMat * vec4(aPos, 1.0);

  vs_out.normal = normalize(aNormalMatrix * aNormal);

  calculateClipping(aModelMatrix * vec4(aPos, 1.0));
}
