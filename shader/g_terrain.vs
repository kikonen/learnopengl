#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl
#include uniform_material_indeces.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;
  flat float tilingX;
  flat uvec2 heightMapTex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;
Material material;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  int materialIndex = entity.u_materialIndex;

  material = u_materials[materialIndex];

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

//  gl_Position = u_projectedMatrix * worldPos;
  gl_Position = pos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;
  vs_out.heightMapTex = material.heightMapTex;

  {
    float x = entity.u_tileX;
    float y = entity.u_tileY;
    float tilingX = material.tilingX;
    float tilingY = material.tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;

    vs_out.tilingX = tilingX;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;

#ifdef USE_TBN
#ifdef USE_NORMAL_TEX
  if (material.normalMapTex.x > 0) {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize(normalMatrix * a_tangent);

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    //const vec3 B = cross(N, T);

    vs_out.tangent = T;
  }
#else
    vs_out.tangent = a_tangent;
#endif
#endif
}
