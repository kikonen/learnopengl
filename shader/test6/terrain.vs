#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in int a_materialIndex;
layout (location = 5) in vec2 a_texCoords;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl
#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat int materialIndex;

  vec4 fragPosLightSpace;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

const mat4 b = {
  {0.5f, 0.0f, 0.0f, 0.0f},
  {0.0f, 0.5f, 0.0f, 0.0f},
  {0.0f, 0.0f, 0.5f, 0.0f},
  {0.5f, 0.5f, 0.5f, 1.0f},
};

void main() {
  gl_Position = u_projectedMatrix * a_modelMatrix * vec4(a_pos, 1.0);

  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoords = a_texCoords * u_materials[a_materialIndex].tiling;

  vs_out.fragPos = (a_modelMatrix * vec4(a_pos, 1.0)).xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewVertexPos = (u_viewMatrix * a_modelMatrix * vec4(a_pos, 1.0)).xyz;

  vs_out.normal = normalize(a_normalMatrix * a_normal);

  calculateClipping(a_modelMatrix * vec4(a_pos, 1.0));

  vs_out.fragPosLightSpace = b * u_lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
}
