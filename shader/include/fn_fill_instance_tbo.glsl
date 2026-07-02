#include "include/tbo_offsets.glsl"
#include "include/struct_instance.glsl"

void fillInstanceTBO(uint instanceIndex)
{
  // 2. Fetch only the parts of the 80-byte Instance we need from TBO
  const int instBase = int(instanceIndex) * INSTANCE_STRIDE_VEC4;

  instance.u_transformRow0 = texelFetch(u_instanceFloatTBO, instBase + INST_SLOT_TRANSFORM_ROW0);
  instance.u_transformRow1 = texelFetch(u_instanceFloatTBO, instBase + INST_SLOT_TRANSFORM_ROW1);
  instance.u_transformRow2 = texelFetch(u_instanceFloatTBO, instBase + INST_SLOT_TRANSFORM_ROW2);

  uvec4 instSlot3 = texelFetch(u_instanceUintTBO, instBase + INST_SLOT_INDICES_AND_FLAGS);
  instance.u_entityIndex   = instSlot3.x;
  instance.u_materialIndex = instSlot3.y;
#ifdef USE_JOINTS
  instance.u_boneBaseIndex = instSlot3.z;
#endif
  instance.u_flags         = instSlot3.w;
}

void fillInstanceDataTBO(uint instanceIndex)
{
  // 2. Fetch only the parts of the 80-byte Instance we need from TBO
  const int instBase = int(instanceIndex) * INSTANCE_STRIDE_VEC4;

  uint data = texelFetch(u_instanceUintTBO, instBase + INST_SLOT_CASE_DATA).x;
  instance.u_data = data;
}
