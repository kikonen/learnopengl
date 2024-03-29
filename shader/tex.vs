#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_clip_plane.glsl
#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_materials.glsl
#include ssbo_material_indeces.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;
  flat uint shapeIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
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
#include fn_calculate_shadow_index.glsl

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  int materialIndex = instance.u_materialIndex;

  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;
  vec3 normal;
#ifdef USE_TBN
  vec3 tangent;
#endif

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((entity.u_flags & ENTITY_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);

    normal = -u_viewFront;
#ifdef USE_TBN
    tangent = u_viewRight;
#endif
  } else if ((entity.u_flags & ENTITY_SPRITE_BIT) != 0) {
    vec4 pos = vec4(u_viewRight * a_pos.x
		    + UP * a_pos.y,
		    1.0);

    worldPos = modelMatrix * pos;

    normal = -u_viewFront;
#ifdef USE_TBN
    tangent = u_viewRight;
#endif
  } else {
    worldPos = modelMatrix * pos;

    normal = normalize(normalMatrix * a_normal);
#ifdef USE_TBN
    tangent = normalize(normalMatrix * a_tangent);
#endif
  }

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;
  vs_out.shapeIndex = instance.u_shapeIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normal;

  calculateClipping(worldPos);

  vs_out.shadowIndex = shadowIndex;
  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = tangent;

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    //const vec3 B = cross(N, T);

    vs_out.tangent = T;
  } else {
    vs_out.tangent = tangent;
  }
#endif
}
