#version 460 core

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

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

precision mediump float;

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  const mat3 normalMatrix = mat3(entity.normalMatrix);
  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = entity.modelMatrix * pos;

  gl_Position = worldPos;

  vs_out.materialIndex = materialIndex;

  vs_out.vertexPos = pos;

  vs_out.normal = normalize(normalMatrix * normal);

  vs_out.scale = vec3(entity.modelMatrix[0][0],
                      entity.modelMatrix[1][1],
                      entity.modelMatrix[2][2]);

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0) {
    const vec3 tangent = vec3(1.0, 0.0, 0.0);

    const vec3 N = vs_out.normal;
    const vec3 T = normalize(normalMatrix * tangent);
    T = normalize(T - dot(T, N) * N);
    const vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
