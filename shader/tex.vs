#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec4 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"
#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_shadow.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_clip_planes.glsl"

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 viewPos;

  flat uint materialIndex;
  flat uint flags;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  // xyz = tangent (view space, not yet Gram-Schmidt'd against interpolated N);
  // w   = handedness
  vec4 tangent;
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
#include "include/fn_calculate_shadow_index.glsl"

void main() {
  instance = GET_INSTANCE;
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
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
                    + u_mainCameraRight.xyz * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
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

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  vs_out.worldPos = worldPos.xyz;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normal;

  calculateClipping(worldPos);

  vs_out.shadowIndex = shadowIndex;
  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

#ifdef USE_TBN
  vs_out.tangent = vec4(tangent, tangentW);
#endif
}
