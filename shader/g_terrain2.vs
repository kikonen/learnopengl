#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_NORMAL_TEX
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

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_out;

//out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

//#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  int materialIndex = entity.materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

//  gl_Position = u_projectedMatrix * worldPos;
  gl_Position = pos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;

  {
    float x = entity.tileX;
    float y = entity.tileY;
    float tilingX = u_materials[materialIndex].tilingX;
    float tilingY = u_materials[materialIndex].tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;

//  calculateClipping(worldPos);

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0)
  {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize(modelMatrix * vec4(a_tangent, 1.0)).xyz;
    T = normalize(T - dot(T, N) * N);
    const vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}