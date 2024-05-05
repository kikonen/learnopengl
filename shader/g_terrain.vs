#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_entity.glsl
#include struct_instance.glsl
#include struct_terrain_tile.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_materials.glsl
#include ssbo_material_indeces.glsl
#include ssbo_terrain_tiles.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  flat uint entityIndex;

#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  flat float tilingX;
  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

ResolvedMaterial material;

TerrainTile tile;

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];
  tile = u_terrainTiles[entity.u_shapeIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

//  gl_Position = u_projectedMatrix * worldPos;
  gl_Position = pos;

  vs_out.entityIndex = entityIndex;
  vs_out.materialIndex = materialIndex;

  vs_out.rangeYmin = tile.u_rangeYmin;
  vs_out.rangeYmax = tile.u_rangeYmax;
  vs_out.heightMapTex = tile.heightMapTex;

  {
    float x = tile.u_tileX;
    float y = tile.u_tileY;
    float tilingX = u_materials[materialIndex].tilingX;
    float tilingY = u_materials[materialIndex].tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;

    vs_out.tilingX = tilingX;
  }

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.vertexPos = a_pos;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    const vec3 N = normalize(vs_out.normal);
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
