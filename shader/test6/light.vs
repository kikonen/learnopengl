#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in int aMaterialIndex;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aNormal;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out vec4 color;
flat out int materialIndex;
out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;
out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  bool clipped = calculateClipping(aModelMatrix * vec4(aPos, 1.0));

  color = aColor;

  materialIndex = aMaterialIndex;
  texCoords = aTexCoords;

  fragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
  normal = normalize(aNormalMatrix * aNormal);
}
