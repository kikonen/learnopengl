#version 460 core

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_socket_transforms.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_FONT_ATLAS_TEX) in vec2 a_atlasCoord;

layout(location = UNIFORM_STENCIL_MODE) uniform int u_stencilMode;

out VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
  flat uint highlightIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Instance instance;
Entity entity;

#include fn_render_outline.glsl

void main()
{
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl

  vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((instance.u_flags & INSTANCE_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_mainCameraRight.xyz * pos.x * entityScale.x
                    + UP * pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = modelMatrix * pos;
  }

  vs_out.atlasCoord = a_atlasCoord;
  vs_out.atlasHandle = entity.u_fontHandle;
  vs_out.highlightIndex = u_selectionMaterialIndex;

  gl_Position = u_projectedMatrix * worldPos;
  renderOutline(u_stencilMode);
}
