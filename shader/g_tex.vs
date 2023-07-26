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
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
#ifdef USE_NORMAL_PATTERN
  vec3 vertexPos;
#endif

  flat uint materialIndex;
  flat uint spriteIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
#ifdef USE_PARALLAX
  vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  int materialIndex = entity.materialIndex;

  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vec4 worldPos;
  vec3 normal;
#ifdef USE_TBN
  vec3 tangent;
#endif

  if ((entity.flags & ENTITY_BILLBOARD_BIT) != 0) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);

    normal = -u_viewFront;
#ifdef USE_TBN
    tangent = u_viewRight;
#endif
  } else {
    worldPos = modelMatrix * vec4(a_pos, 1.0);

    normal = normalize(normalMatrix * a_normal);
#ifdef USE_TBN
    tangent = normalize(normalMatrix * a_tangent);
#endif
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.spriteIndex = entity.spriteIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif
#ifdef USE_NORMAL_PATTERN
  vs_out.vertexPos = a_pos;
#endif

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normal;

  calculateClipping(worldPos);

#ifdef USE_TBN
  if (u_materials[materialIndex].normalMapTex >= 0 || u_materials[materialIndex].heightMapTex >= 0) {
    const vec3 N = normal;
    vec3 T = tangent;

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    vs_out.tangent = T;

#ifdef USE_PARALLAX
    const vec3 B = cross(N, T);
    const mat3 invTBN = transpose(mat3(T, B, N));
    vs_out.viewTangentPos  = invTBN * u_viewWorldPos;
    vs_out.tangentPos  = invTBN * worldPos.xyz;
#endif
  }
#endif
}
