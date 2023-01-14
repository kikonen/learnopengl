#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

#ifdef USE_ALPHA
out VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} vs_out;
#else
out VS_OUT {
  flat vec4 objectID;
} vs_out;
#endif


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Entity entity = u_entities[int(gl_BaseInstance) + gl_InstanceID];
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.objectID = entity.objectID;

#ifdef USE_ALPHA
  int materialIndex = int(entity.materialIndex);
  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;
#endif
}
