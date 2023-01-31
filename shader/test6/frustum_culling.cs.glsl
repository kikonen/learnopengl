#version 460 core

layout (local_size_x = 1, local_size_y = 1) in;

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
};

struct PerformanceCounters {
  uint drawCount;
  uint skipCount;
};

layout(location = UNIFORM_DRAW_PARAMETERS_INDEX) uniform uint u_drawParametersIndex;

layout (binding = SSBO_DRAW_COMMANDS, std430) restrict buffer DrawCommandSSBO
{
  DrawIndirectCommand u_commands[];
};

layout (binding = SSBO_DRAW_PARAMETERS, std430) readonly buffer DrawParametersSSBO
{
  DrawParameters u_params[];
};

layout (binding = SSBO_PERFORMANCE_COUNTERS, std430) writeonly buffer PerformanceCountersSSBO
{
  PerformanceCounters u_counters;
};


const float EXPAND_X = 1.0;
const float EXPAND_Y = 1.0;

void main(void) {
  const DrawParameters param = u_params[u_drawParametersIndex];
  const uint baseIndex = param.baseIndex;
  const DrawIndirectCommand cmd = u_commands[baseIndex + gl_GlobalInvocationID.x];
  const uint baseInstance = param.drawType == DRAW_TYPE_ELEMENTS ? cmd.baseInstance_or_pad : cmd.baseVertex_or_baseInstance;
  const Entity entity = u_entities[baseInstance];

  bool visible = true;

  if ( !((entity.flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)) {
    const vec4 pos = u_projectedMatrix *
      entity.modelMatrix *
      vec4(entity.volumeCenter, 1.0);

    const vec4 radiusPos = u_projectedMatrix *
      entity.modelMatrix *
      vec4(entity.volumeCenter + vec3(entity.volumeRadius, entity.volumeRadius, entity.volumeRadius), 1.0);

    const float radius = length(vec3(radiusPos) - vec3(pos));

    visible = (abs(pos.x) - radius) < (pos.w * EXPAND_X) &&
      (abs(pos.y) - radius) < (pos.w * EXPAND_Y);
  }

  if (visible) {
    atomicAdd(u_counters.drawCount, 1);
    u_commands[baseIndex + gl_GlobalInvocationID.x].instanceCount = 1;
  } else {
    atomicAdd(u_counters.skipCount, 1);
  }
}
