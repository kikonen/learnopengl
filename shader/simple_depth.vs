#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#ifdef USE_ALPHA
#include uniform_material_indeces.glsl
#endif

layout(location = UNIFORM_SHADOW_MAP_INDEX) uniform uint u_shadowIndex;

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} vs_out;
#endif

void main()
{
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  const uint shadowIndex = 0; //u_shadowIndex;
  const vec4 worldPos = modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_shadowProjectedMatrix[shadowIndex] * worldPos;

#ifdef USE_ALPHA
  int materialIndex = entity.materialIndex;
  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;
#endif
}
