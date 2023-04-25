#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_material_indeces.glsl
#include uniform_clip_planes.glsl

layout(location = UNIFORM_STENCIL_MODE) uniform int u_stencilMode;

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint highlightIndex;
} vs_out;
#else
out VS_OUT {
  flat uint highlightIndex;
} vs_out;
#endif

out float gl_ClipDistance[CLIP_COUNT];

const float SCALE = 1.024;
const mat4 SCALE_MATRIX = mat4(SCALE, 0, 0, 0,
                               0, SCALE, 0, 0,
                               0, 0, SCALE, 0,
                               0, 0,     0, 1);

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
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

  if ((entity.flags & ENTITY_BILLBOARD_BIT) == ENTITY_BILLBOARD_BIT) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = vec3(entity.worldScale[0] * scale,
                            entity.worldScale[1] * scale,
                            entity.worldScale[2] * scale);

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + u_viewUp * a_pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = modelMatrix * scaleMatrix * pos;
  }

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  int materialIndex = entity.materialIndex;
  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;
#endif

  vs_out.highlightIndex = entity.highlightIndex;

  calculateClipping(worldPos);
}
