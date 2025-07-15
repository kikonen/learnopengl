#version 460 core

layout (local_size_x = CS_GROUP_X, local_size_y = CS_GROUP_Y) in;

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl

// struct DrawElementsIndirectCommand
// {
//   uint count;
//   uint instanceCount;
//   uint firstIndex;
//   uint baseVertex;
//   uint baseInstance;
// };

// struct DrawArraysIndirectCommand
// {
//   uint vertexCount;
//   uint instanceCount;
//   uint firstVertex;
//   uint baseInstance;
// };

// HACK KI deal both array/element with same CS in same strided buffer
struct DrawIndirectCommand
{
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint baseVertex_or_baseInstance;
  uint baseInstance_or_pad;
};

struct DrawParameters {
  uint baseIndex;
  uint drawType;
  uint drawCount;
};

#ifdef FRUSTUM_DEBUG
struct PerformanceCounters {
  uint drawCount;
  uint skipCount;
};

layout (binding = SSBO_PERFORMANCE_COUNTERS, std430) writeonly buffer PerformanceCountersSSBO
{
  PerformanceCounters u_counters;
};
#endif

layout(location = UNIFORM_DRAW_PARAMETERS_INDEX) uniform uint u_drawParametersIndex;

layout (binding = SSBO_DRAW_COMMANDS, std430) restrict buffer DrawCommandSSBO
{
  DrawIndirectCommand u_commands[];
};

layout (binding = SSBO_DRAW_PARAMETERS, std430) readonly buffer DrawParametersSSBO
{
  DrawParameters u_params[];
};

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

float getSignedDistanceToPlane(in vec4 plane, in vec3 p)
{
  return dot(plane.xyz, p) - plane.w;
}

bool isOnOrForwardPlane(in vec4 plane, in vec3 p, in float radius)
{
  return getSignedDistanceToPlane(plane, p) >= -radius;
}

bool isOnFrustum(in vec4 volume)
{
  const vec3 p = volume.xyz;
  const float radius = volume.w;

  return
    isOnOrForwardPlane(u_frustum[0], p, radius) &&
    isOnOrForwardPlane(u_frustum[1], p, radius) &&
    isOnOrForwardPlane(u_frustum[2], p, radius) &&
    isOnOrForwardPlane(u_frustum[3], p, radius) &&
    isOnOrForwardPlane(u_frustum[4], p, radius) &&
    isOnOrForwardPlane(u_frustum[5], p, radius);
};

void main(void) {
  // NOTE KI this is valid *ONLY* if local_y == local_z == 1
  const uint drawIndex = gl_GlobalInvocationID.x + (CS_GROUP_X - 1) * (gl_GlobalInvocationID.y - 1);

  const DrawParameters param = u_params[u_drawParametersIndex];
  //if (drawIndex > param.drawCount) return;

  const uint baseIndex = param.baseIndex;
  const DrawIndirectCommand cmd = u_commands[baseIndex + drawIndex];
  const uint baseInstance = param.drawType == DRAW_TYPE_ELEMENTS ? cmd.baseInstance_or_pad : cmd.baseVertex_or_baseInstance;

  entity = u_entities[baseInstance];

  const bool skip = cmd.instanceCount > 0;
  bool visible = (entity.u_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT;

  if (!skip && !visible) {
    visible = isOnFrustum(entity.u_volume);
  }

#ifdef FRUSTUM_DEBUG
  if (skip) {
    atomicAdd(u_counters.drawCount, 1);
  } else if (visible) {
    atomicAdd(u_counters.drawCount, 1);
    u_commands[baseIndex + drawIndex].instanceCount = 1;
  } else {
    atomicAdd(u_counters.skipCount, 1);
  }
#else
  if (!skip && visible) {
    u_commands[baseIndex + drawIndex].instanceCount = 1;
  }
#endif
}
