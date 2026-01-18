#version 460 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

#include "include/cull_multiview_types.glsl"

layout(std430, binding = SSBO_CULL_MESH_INFOS) readonly buffer MeshInfosSSBO {
  MeshInfo u_meshes[];
};

layout(std430, binding = SSBO_DRAW_COMMANDS) writeonly buffer DrawCommands {
  DrawElementsIndirectCommand u_drawCommands[];
};

layout(std430, binding = SSBO_CULL_VISIBLE_COUNTS) buffer VisibleCountsSSBO {
  uint u_visibleCounts[];
};

layout(location = UNIFORM_CULL_MESH_COUNT) uniform uint u_meshCount;

void main() {
  uint meshId = gl_GlobalInvocationID.x;

  if (meshId >= u_meshCount) return;

  MeshInfo mesh = u_meshes[meshId];
  uint visibleCount = u_visibleCounts[meshId];

  u_drawCommands[meshId] = DrawElementsIndirectCommand(
    mesh.indexCount,
    visibleCount,           // instance count from culling pass
    mesh.firstIndex,
    mesh.baseVertex,
    mesh.outputOffset       // baseInstance = offset into VisibleInstances
  );
}
