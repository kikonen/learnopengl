#version 460 core

#define USE_BONES_NORMAL 1

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include tech_skinned_mesh_data.glsl

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_clip_plane.glsl
#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_socket_transforms.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl
#include uniform_debug.glsl

out VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
#ifdef USE_NORMAL_PATTERN
  vec3 vertexPos;
#endif

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

#ifdef USE_BONES
#ifdef USE_DEBUG
  flat uint boneBaseIndex;
  flat uvec4 boneIndex;
  flat vec4 boneWeight;
  vec3 boneColor;
#endif
#endif
#ifdef USE_DEBUG
  flat uint socketBaseIndex;
  flat uint socketIndex;
#endif
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Instance instance;
Entity entity;

#include fn_calculate_clipping.glsl

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  vec4 pos = vec4(a_pos, 1.0);

  vec4 worldPos;
  vec3 normal;
#ifdef USE_TBN
  vec3 tangent;
#endif

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((instance.u_flags & INSTANCE_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    // TODO KI "vs_out.vertexPos" might be incorrect
    worldPos = vec4(entityPos
                    + u_viewRight * pos.x * entityScale.x
                    + UP * pos.y * entityScale.y,
                    1.0);

    normal = -u_viewFront;
#ifdef USE_TBN
    tangent = u_viewRight;
#endif
  } else {
    normal = a_normal;
#ifdef USE_TBN
    tangent = a_tangent;
#endif

    #include tech_skinned_mesh_skin.glsl

    worldPos = modelMatrix * pos;
    normal = normalize(normalMatrix * normal);
#ifdef USE_TBN
    tangent = normalize(normalMatrix * tangent);
#endif
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

#ifdef USE_BONES
#ifdef USE_DEBUG
  if (u_debugBoneWeight) {
    vs_out.boneBaseIndex = entity.u_boneBaseIndex;
    vs_out.boneIndex = a_boneIndex;
    vs_out.boneWeight = a_boneWeight;

    uint tbi = u_debugBoneIndex;
    uvec4 bi = a_boneIndex;
    vec4 wi = a_boneWeight;

    // tbi = 4;
    // bi = uvec4(4, 0, 0, 0);
    // wi = vec4(1, 0, 0, 0);

    float w = 0;
    if (bi.x == tbi && wi.x > 0) w += wi.x;
    if (bi.y == tbi && wi.y > 0) w += wi.y;
    if (bi.z == tbi && wi.z > 0) w += wi.z;
    if (bi.w == tbi && wi.w > 0) w += wi.w;

    vec3 shade = vec3(0);
    if (w > 0.9) {
      shade = vec3(1.0, 0, 0);
    } else if (w > 0.5) {
      shade = vec3(0, 1.0, 1.0);
    } else if (w > 0.25) {
      shade = vec3(0, 1.0, 0);
    } else if (w > 0.0) {
      shade = vec3(0, 0, 1.0);
    }

    vs_out.boneColor = shade;
  }
#endif
#endif

#ifdef USE_DEBUG
  vs_out.socketBaseIndex = entity.u_socketBaseIndex;
  vs_out.socketIndex = instance.u_socketIndex;
#endif

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

#ifdef USE_NORMAL_PATTERN
  vs_out.vertexPos = pos.xyz;
#endif

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normal;

  calculateClipping(worldPos);

#ifdef USE_TBN

  if (u_materials[materialIndex].normalMapTex.x > 0 || u_materials[materialIndex].parallaxDepth > 0)
  {
    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    //tangent = normalize(tangent - dot(tangent, normal) * normal);

    const vec3 bitangent = cross(normal, tangent);

    vs_out.tbn = mat3(tangent, bitangent, normal);

#ifdef USE_PARALLAX
    const mat3 invTBN = transpose(vs_out.tbn);
    vs_out.viewTangentPos  = invTBN * u_viewWorldPos;
    vs_out.tangentPos  = invTBN * worldPos.xyz;
#endif
  }
#endif

  // HACK KI for primitive GL_POINTS
  gl_PointSize = u_materials[materialIndex].layersDepth;
}
