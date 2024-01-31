#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include ssbo_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl
#include ssbo_material_indeces.glsl

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

Entity entity;
Material material;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  int materialIndex = entity.u_materialIndex;

  material = u_materials[materialIndex];

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

  gl_Position = pos;

  vs_out.modelMatrix = modelMatrix;
  vs_out.rangeYmin = entity.u_rangeYmin;
  vs_out.rangeYmax = entity.u_rangeYmax;
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
}
