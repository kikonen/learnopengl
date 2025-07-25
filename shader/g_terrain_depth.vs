#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl

out VS_OUT {
  flat mat4 modelMatrix;

  vec3 worldPos;
  vec2 texCoord;
  vec3 vertexPos;

  flat float tilingX;
  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;
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
  tile = u_terrainTiles[instane.u_data];

  #include var_entity_model_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

  gl_Position = pos;

  vs_out.modelMatrix = modelMatrix;

  vs_out.rangeYmin = tile.u_rangeYmin;
  vs_out.rangeYmax = tile.u_rangeYmax;
  vs_out.heightMapTex = tile.heightMapTex;

  {
    float x = tile.u_tileX;
    float y = tile.u_tileY;
    float tilingX = u_materials[materialIndex].tilingX * entity.tilingX;
    float tilingY = u_materials[materialIndex].tilingY * entity.tilingY;
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
}
