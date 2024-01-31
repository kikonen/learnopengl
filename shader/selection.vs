#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_entity.glsl

#include ssbo_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_material_indeces.glsl

layout(location = UNIFORM_STENCIL_MODE) uniform int u_stencilMode;

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint shapeIndex;
  flat uint highlightIndex;
} vs_out;
#else
out VS_OUT {
  flat uint highlightIndex;
} vs_out;
#endif

const float SCALE = 1.024;
const mat4 SCALE_MATRIX = mat4(SCALE, 0, 0, 0,
                               0, SCALE, 0, 0,
                               0, 0, SCALE, 0,
                               0, 0,     0, 1);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  float scale;
  mat4 scaleMatrix;

  if (u_stencilMode == STENCIL_MODE_MASK) {
    scale = 1.0;
    scaleMatrix = mat4(1.0);
  } else if (u_stencilMode == STENCIL_MODE_HIGHLIGHT) {
    scale = SCALE;
    scaleMatrix = SCALE_MATRIX;
  } else {
    scale = 1.5;
    scaleMatrix = mat4(scale, 0, 0, 0,
                       0, scale, 0, 0,
                       0, 0, scale, 0,
                       0, 0,     0, 1);
  }

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((entity.u_flags & ENTITY_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = vec3(entity.u_worldScale[0] * scale,
                            entity.u_worldScale[1] * scale,
                            entity.u_worldScale[2] * scale);

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);
  } else if ((entity.u_flags & ENTITY_SPRITE_BIT) != 0) {
    vec4 pos = vec4(u_viewRight * a_pos.x
		    + UP * a_pos.y,
		    1.0);

    worldPos = modelMatrix * pos;
  } else {
    worldPos = modelMatrix * scaleMatrix * pos;
  }

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  int materialIndex = entity.u_materialIndex;
  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vs_out.materialIndex = materialIndex;
  vs_out.shapeIndex = entity.u_shapeIndex;
  vs_out.texCoord = a_texCoord;
#endif

  vs_out.highlightIndex = entity.u_highlightIndex;
}
