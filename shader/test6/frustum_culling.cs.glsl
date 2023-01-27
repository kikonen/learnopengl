#version 460 core

layout (local_size_x = 8, local_size_y = 4) in;

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
  uint u_baseIndex;
  uint u_drawType;
};

layout(location = UNIFORM_DRAW_PARAMETERS_INDEX) uniform uint u_drawParametersIndex;

layout (binding = SSBO_DRAW_COMMANDS, std430) buffer DrawCommandSSBO
{
  DrawIndirectCommand u_commands[];
};

layout (binding = SSBO_DRAW_PARAMETERS, std430) buffer DrawParametersSSBO
{
  DrawParameters u_params[];
};

const float EXPAND_X = 1.0;
const float EXPAND_Y = 1.0;

void main(void) {
  const DrawParameters param = u_params[u_drawParametersIndex];
  const uint baseIndex = param.u_baseIndex;
  const DrawIndirectCommand cmd = u_commands[baseIndex + gl_GlobalInvocationID.x];
  const uint baseInstance = param.u_drawType == DRAW_TYPE_ELEMENTS ? cmd.baseInstance_or_pad : cmd.baseVertex_or_baseInstance;
  const Entity entity = u_entities[baseInstance];

  const vec4 pos = u_projectedMatrix *
    entity.modelMatrix *
    vec4(entity.volumeCenter, 1.0);

  const vec4 radiusPos = u_projectedMatrix *
    entity.modelMatrix *
    vec4(entity.volumeCenter + vec3(entity.volumeRadius, entity.volumeRadius, entity.volumeRadius), 1.0);

  const float radius = length(vec3(radiusPos) - vec3(pos));

  if ((abs(pos.x) - radius) < (pos.w * EXPAND_X) &&
      (abs(pos.y) - radius) < (pos.w * EXPAND_Y))
    {
      u_commands[baseIndex + gl_GlobalInvocationID.x].instanceCount = 1;
    }
}
