#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;
layout (location = ATTR_FONT_ATLAS_TEX) in vec2 a_atlasCoord;

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

out VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
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

    worldPos = vec4(entityPos
                    + u_mainCameraRight * pos.x * entityScale.x
                    + UP * pos.y * entityScale.y,
                    1.0);

    normal = -u_mainCameraFront;
#ifdef USE_TBN
    tangent = u_mainCameraRight;
#endif
  } else {
    worldPos = modelMatrix * pos;

    normal = normalize(normalMatrix * a_normal);
#ifdef USE_TBN
    tangent = normalize(normalMatrix * a_tangent);
#endif
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  vs_out.atlasCoord = a_atlasCoord;
  vs_out.atlasHandle = entity.u_fontHandle;

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

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
    const mat3 invTBN = transpose(tbn);
    vs_out.viewTangentPos  = invTBN * u_cameraPos;
    vs_out.tangentPos  = invTBN * worldPos.xyz;
#endif
  }
#endif
}
