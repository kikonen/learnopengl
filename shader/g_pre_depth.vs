#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"

#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Instance instance;
Entity entity;

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"

void main()
{
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"

  vec4 worldPos;

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((instance.u_flags & INSTANCE_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_mainCameraRight.xyz * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = modelMatrix * vec4(a_pos, 1.0);
  }

  gl_Position = u_projectedMatrix * worldPos;
}
