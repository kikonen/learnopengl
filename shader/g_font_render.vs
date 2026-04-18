#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec4 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;
layout (location = ATTR_FONT_ATLAS_TEX) in vec2 a_atlasCoord;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"
#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_clip_planes.glsl"

out VS_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
#ifdef USE_TBN_FS_RECONSTRUCT
  vec4 tangent;
#else
  mat3 tbn;
#endif
#endif
#if defined(USE_PARALLAX) && !defined(USE_TBN_FS_RECONSTRUCT)
  vec3 tangentPos;
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

#include "include/fn_calculate_clipping.glsl"

void main() {
  instance = GET_INSTANCE;
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;
  vec3 normal;
#ifdef USE_TBN
  vec3 tangent;
  // handedness, default for billboard
  float tangentW = 1.0;
#endif

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((instance.u_flags & INSTANCE_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_mainCameraRight.xyz * pos.x * entityScale.x
                    + UP * pos.y * entityScale.y,
                    1.0);

    // constant view-space answer vec3(0,0,1) for the billboard normal
    // and vec3(1,0,0) for its tangent (since billboards face the camera by definition).
    normal = vec3(0, 0, 1);
#ifdef USE_TBN
    tangent = vec3(1, 0, 0);
#endif
  } else {
    worldPos = modelMatrix * pos;

    normal = normalize(viewNormalMatrix * DECODE_A_NORMAL(a_normal));
#ifdef USE_TBN
    tangent = normalize(viewNormalMatrix * DECODE_A_TANGENT(a_tangent));
    tangentW = DECODE_A_TANGENT_W(a_tangent);
#endif
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  vs_out.atlasCoord = a_atlasCoord;
  vs_out.atlasHandle = entity.u_fontHandle;

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normal;

  calculateClipping(worldPos);

#ifdef USE_TBN
#ifdef USE_TBN_FS_RECONSTRUCT
  vs_out.tangent = vec4(tangent, tangentW);
#else
  {
    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    tangent = normalize(tangent - dot(tangent, normal) * normal);

    const vec3 bitangent = cross(normal, tangent) * tangentW;

    vs_out.tbn = mat3(tangent, bitangent, normal);

#ifdef USE_PARALLAX
    const mat3 invTBN = transpose(vs_out.tbn);
    vs_out.tangentPos  = invTBN * vs_out.viewPos.xyz;
#endif
  }
#endif
#endif
}
