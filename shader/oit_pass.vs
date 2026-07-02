#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/tech_skinned_mesh_data.glsl"

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"
#include "include/tbo_materials.glsl"

#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_clip_planes.glsl"

out VS_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Instance instance;
Entity entity;
ResolvedMaterial material;

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"
#include "include/fn_fill_material_tbo.glsl"

#include "include/fn_calculate_clipping.glsl"
#include "include/fn_mod.glsl"

void main() {
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;
  vec3 normal;

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
  } else {
    normal = DECODE_A_NORMAL(a_normal);

    #include "include/tech_skinned_mesh_skin.glsl"
    #include "include/apply_mod.glsl"

    worldPos = modelMatrix * pos;

    normal = normalize(viewNormalMatrix * normal);
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;
  // view-space normal (billboard => faces camera). No tangent => no normal mapping in OIT.
  vs_out.normal = normal;

  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;

  fillMaterialTilingTBO(materialIndex);

  vs_out.texCoord.x = a_texCoord.x * material.tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * material.tilingY * entity.tilingY;

  calculateClipping(worldPos);
}
