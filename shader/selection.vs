#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
#ifdef USE_ALPHA
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif

#include tech_skinned_mesh_data.glsl

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_socket_transforms.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl

layout(location = UNIFORM_STENCIL_MODE) uniform int u_stencilMode;

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint flags;
  flat uint highlightIndex;
} vs_out;
#else
out VS_OUT {
  flat uint highlightIndex;
} vs_out;
#endif

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Instance instance;
Entity entity;

#include fn_mod_flag_wave.glsl

void main() {
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
    vec3 entityScale = vec3(entity.u_worldScale[0],
                            entity.u_worldScale[1],
                            entity.u_worldScale[2]);

    worldPos = vec4(entityPos
                    + u_cameraRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);
  } else {
    #include tech_skinned_mesh_skin.glsl

#ifdef USE_FLAG_WAVE
    mod_flag_wave(pos);
#endif

    worldPos = modelMatrix * pos;
  }

  gl_Position = u_projectedMatrix * worldPos;

#ifdef USE_ALPHA
  const uint materialIndex = instance.u_materialIndex;

  vs_out.materialIndex = materialIndex;
  vs_out.flags = instance.u_flags;
  vs_out.texCoord = a_texCoord;
#endif

  vs_out.highlightIndex = entity.u_highlightIndex;
}
