#version 460 core

layout (local_size_x = CS_GROUP_X, local_size_y = CS_GROUP_Y) in;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

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

void main(void) {
  // NOTE KI this is valid *ONLY* if local_y == local_z == 1
  const uint drawIndex = gl_GlobalInvocationID.x + CS_GROUP_X * (gl_GlobalInvocationID.y - 1);

  const DrawParameters param = u_params[u_drawParametersIndex];
  //if (drawIndex > param.drawCount) return;

  const uint baseIndex = param.baseIndex;
  const DrawIndirectCommand cmd = u_commands[baseIndex + drawIndex];
  const uint baseInstance = param.drawType == DRAW_TYPE_ELEMENTS ? cmd.baseInstance_or_pad : cmd.baseVertex_or_baseInstance;

  const Entity entity = u_entities[baseInstance];
  #include var_entity_model_matrix.glsl

  const bool skip = cmd.instanceCount > 0;
  bool visible = (entity.flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT;

  // https://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/
  if (!skip && !visible) {
    const vec3 volumeCenter = entity.volume.xyz;
    const float volumeRadius = entity.volume.a;

    const mat4 projectedModel = u_projectedMatrix * modelMatrix;

    const vec4 pos = projectedModel *
      vec4(volumeCenter, 1.0);

    const vec4 radiusPos = projectedModel *
      vec4(volumeCenter + vec3(volumeRadius, 0, 0), 1.0);

    const float radius = length(vec3(radiusPos) - vec3(pos));

    const float w = pos.w * 1.0;

    visible = -w <= pos.x + radius && pos.x - radius <= w &&
      -w <= pos.y + radius && pos.y - radius <= w &&
      -w <= pos.z + radius && pos.z - radius <= w;
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
