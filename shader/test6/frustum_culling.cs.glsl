#version 460 core

layout (local_size_x = 1) in;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

struct CandidateDraw
{
  uint baseInstance;
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint baseVertex_or_baseInstance;
  uint baseInstance_or_pad;
};

struct DrawElementsIndirectCommand
{
  uint count;
  uint instanceCount;
  uint firstIndex;
  uint baseVertex;
  uint baseInstance;
};

struct DrawArraysIndirectCommand
{
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint baseInstance;
};

// HACK KI deal both array/element with same CS in same strided buffer
struct DrawIndirectCommand
{
  uint vertexCount;
  uint instanceCount;
  uint firstVertex;
  uint baseVertex_or_baseInstance;
  uint baseInstance_or_pad;
};

layout (std430, binding = SSBO_CANDIDATE_DRAWS) readonly buffer CandidateDrawSSBO {
  CandidateDraw u_candidates[];
};

layout (binding = SSBO_DRAW_COMMANDS, std430) writeonly buffer DrawCommandSSBO
{
  DrawIndirectCommand u_commands[];
};

layout (binding = SSBO_DRAW_COMMAND_COUNTER) buffer CommandCounterSSBO
{
  uint u_commandCounter;
};

const float EXPAND_X = 1.0;
const float EXPAND_Y = 1.0;

void main(void) {
  const CandidateDraw draw = u_candidates[gl_GlobalInvocationID.x];
  const Entity entity = u_entities[draw.baseInstance];

  const vec4 pos = u_projectedMatrix *
    entity.modelMatrix *
    vec4(entity.volumeCenter, 1.0);

const vec4 radiusPos = u_projectedMatrix *
  entity.modelMatrix *
  vec4(entity.volumeCenter + vec3(entity.volumeRadius, .0, .0), 1.0);

  const float radius = length(vec3(radiusPos) - vec3(pos));

  if ((abs(pos.x) - radius) < (pos.w * EXPAND_X) &&
      (abs(pos.y) - radius) < (pos.w * EXPAND_Y))
    {
      uint idx = atomicAdd(u_commandCounter, 1);

      u_commands[idx].vertexCount = draw.vertexCount;
      u_commands[idx].instanceCount = draw.instanceCount;
      u_commands[idx].firstVertex = draw.firstVertex;
      u_commands[idx].baseVertex_or_baseInstance = draw.baseVertex_or_baseInstance;
      u_commands[idx].baseInstance_or_pad = draw.baseInstance_or_pad;
    }
}
