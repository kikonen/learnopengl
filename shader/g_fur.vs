#version 460 core

#define USE_LAYERS

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec4 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/tech_skinned_mesh_data.glsl"

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"
#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

out VS_OUT {
  flat mat4 modelMatrix;
  flat mat3 viewNormalMatrix;

  vec3 objectNormal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float layersDepth;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

void main() {
  instance = GET_INSTANCE;
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  vec4 pos = vec4(a_pos, 1.0);

  vec3 normal;
#ifdef USE_TBN
  vec3 tangent;
  // handedness, default for billboard
  float tangentW = 1.0;
#endif

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if (false) {
  } else {
    normal = DECODE_A_NORMAL(a_normal);
    // // TODO KI *WHY* when rotated 180 aorund Y this makes it correct
    // normal = -normal;
#ifdef USE_TBN
    tangent = DECODE_A_TANGENT(a_tangent);
    tangentW = DECODE_A_TANGENT_W(a_tangent);
#endif

    #include "include/tech_skinned_mesh_skin.glsl"
  }

  gl_Position = pos;

  vs_out.modelMatrix = modelMatrix;
  vs_out.viewNormalMatrix = viewNormalMatrix;

  const uint materialIndex = instance.u_materialIndex;

  vs_out.materialIndex = materialIndex;

  vs_out.layers = readMaterial_layers(materialIndex);
  vs_out.layersDepth = readMaterial_layersDepth(materialIndex);

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  vs_out.objectNormal = DECODE_A_NORMAL(a_normal);
}
