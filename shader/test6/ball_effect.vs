#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
layout (location = ATTR_TEX) in vec2 a_texCoord;
layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;

#include struct_material.glsl
#include struct_texture.glsl
#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  vec4 fragPosLightSpace;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_clipping.glsl

void main() {
  int materialIndex = int(a_materialIndex);
  vec4 worldPos = a_modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord * u_materials[materialIndex].tiling;

  vs_out.fragPos = worldPos.xyz;
  vs_out.normal = normalize(a_normalMatrix * a_normal);

  calculateClipping(worldPos);

  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
}
