#version 460 core

#include include/struct_material.glsl
#include include/struct_entity.glsl

#include include/ssbo_entities.glsl
#include include/uniform_matrices.glsl
#include include/uniform_data.glsl
#include include/ssbo_materials.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 normal;
  vec3 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_out;

// TODO KI y = -1.0 fixes sprite.gs, but why?!?
const vec4 pos = vec4(0.0, -1.0, 0.0, 1.0);
const vec3 normal = vec3(0.0, 0.0, 1.0);
const vec3 tangent = vec3(1.0, 0.0, 0.0);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include include/var_entity_model_matrix.glsl
  #include include/var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;
  const vec4 worldPos = modelMatrix * pos;

  gl_Position = worldPos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;

  vs_out.vertexPos = pos.xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * normal;

  vs_out.scale = entity.u_worldScale.xyz;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize(normalMatrix * tangent);

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    //const vec3 B = cross(N, T);

    vs_out.tangent = T;
  }
#endif
}
