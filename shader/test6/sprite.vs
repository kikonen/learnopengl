#version 450 core

#include constants.glsl

layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;

#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  vec3 normal;
  vec4 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_out;

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec4 pos = vec4(0.0, -1.0, 0.0, 1.0);
const vec3 normal = vec3(0.0, 0.0, 1.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

void main() {
  int materialIndex = int(a_materialIndex);
  vec4 worldPos = a_modelMatrix * pos;

  gl_Position = worldPos;

  vs_out.materialIndex = materialIndex;

  vs_out.vertexPos = pos;

  vs_out.normal = normalize(a_normalMatrix * normal);

  vs_out.scale = vec3(a_modelMatrix[0][0],
                      a_modelMatrix[1][1],
                      a_modelMatrix[2][2]);

#ifdef USE_NORMAL_TEX
  {
    vec3 tangent = vec4(1.0, 0.0, 0.0);

    vec3 N = vs_out.normal;
    vec3 T = normalize(a_normalMatrix * tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
