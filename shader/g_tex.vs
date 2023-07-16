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
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 normal;
  vec2 texCoord;
#ifdef USE_NORMAL_PATTERN
  vec3 vertexPos;
#endif

  flat uint materialIndex;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  int materialIndex = entity.materialIndex;

  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  vec3 normal;

  if ((entity.flags & ENTITY_BILLBOARD_BIT) == ENTITY_BILLBOARD_BIT) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.worldScale;

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + u_viewUp * a_pos.y * entityScale.y,
                    1.0);

    normal = -u_viewFront;
  } else {
    worldPos = modelMatrix * pos;

    normal = normalize(normalMatrix * a_normal);
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;

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

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0) {
    vec3 tangent;
    if ((entity.flags & ENTITY_BILLBOARD_BIT) == ENTITY_BILLBOARD_BIT) {
      tangent = u_viewRight;
    } else {
      tangent = normalize((modelMatrix * vec4(a_tangent, 1.0)).xyz);
    }

    const vec3 N = normalize(vs_out.normal);
    vec3 T = tangent;

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    const vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
