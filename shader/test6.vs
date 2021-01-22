#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 4) in float aMaterialIndex;
layout (location = 6) in mat4 aInstanceMatrix;

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};

uniform mat3 normalMat;
uniform mat4 model;
uniform bool drawInstanced;

out VS_OUT {
  flat float materialIndex;
  vec3 fragPos;
  vec3 normal;
} vs_out;

void main() {
  if (drawInstanced) {
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  vs_out.materialIndex = aMaterialIndex;

  vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
  vs_out.normal = normalMat * aNormal;
}
