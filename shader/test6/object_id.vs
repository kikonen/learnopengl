#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 13) in vec4 aObjectID;

#include uniform_matrices.glsl

out VS_OUT {
  flat vec4 objectID;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  vs_out.objectID = aObjectID;
}
