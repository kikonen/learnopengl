#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include ssbo_entities.glsl
#include uniform_matrices.glsl
#include ssbo_materials.glsl
#include uniform_data.glsl


out VS_OUT {
  vec4 glp;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;
  const vec3 normal = normalize(normalMatrix * a_normal);

  vs_out.glp = u_projectedMatrix * worldPos;
  gl_Position = vs_out.glp;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  vs_out.worldPos = worldPos.xyz;

  vs_out.normal = normal;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    const vec3 N = normal;
    vec3 T = normalize(normalMatrix * a_tangent);

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    //const vec3 B = cross(N, T);

    vs_out.tangent = T;
  } else {
    vs_out.tangent = a_tangent;
  }
#endif
}
