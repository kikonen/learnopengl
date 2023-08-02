#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#ifdef USE_ALPHA
#include uniform_material_indeces.glsl
#endif

layout(location = UNIFORM_SHADOW_MAP_INDEX) uniform uint u_shadowIndex;

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint shapeIndex;
} vs_out;
#endif

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

void main()
{
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  vec4 worldPos;

  if ((entity.flags & ENTITY_BILLBOARD_BIT) != 0) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = modelMatrix * vec4(a_pos, 1.0);
  }


  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  int materialIndex = entity.materialIndex;
  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vs_out.materialIndex = materialIndex;
  vs_out.shapeIndex = entity.shapeIndex;
  vs_out.texCoord = a_texCoord;
#endif
}
