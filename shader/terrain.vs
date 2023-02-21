#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_lights.glsl
#include struct_material.glsl
#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;

  if (false) {
    float x = u_materials[materialIndex].tileX;
    float y = u_materials[materialIndex].tileY;
    float tilingX = u_materials[materialIndex].tilingX;
    float tilingY = u_materials[materialIndex].tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;
  } else {
    vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
    vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = pos;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;

  calculateClipping(worldPos);

  vs_out.shadowPos = u_shadowMatrix * worldPos;
}
