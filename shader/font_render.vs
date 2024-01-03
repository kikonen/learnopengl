#version 460 core

#include uniform_data.glsl

layout (location = ATTR_POS) in vec3 a_pos;
//layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

layout(location = UNIFORM_MODEL_MATRIX) uniform mat4 u_modelMatrix;
layout(location = UNIFORM_MATERIAL_INDEX) uniform uint u_materialIndex;

out VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main()
{
  vs_out.texCoord = a_texCoord;
  vs_out.materialIndex = u_materialIndex;

  gl_Position = u_projectedMatrix * u_modelMatrix * vec4(a_pos, 1.0);
}
