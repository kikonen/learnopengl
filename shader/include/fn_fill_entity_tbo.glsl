#include "include/struct_entity.glsl"

void fillEntityTBO(uint entityIndex)
{
  // Each Entity spans 10 slots of vec4
  const int entBase = int(entityIndex) * ENTITY_STRIDE_VEC4;

  entity.u_modelMatrixRow0 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_MODEL_ROW0);
  entity.u_modelMatrixRow1 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_MODEL_ROW1);
  entity.u_modelMatrixRow2 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_MODEL_ROW2);

  entity.u_normalMatrix0 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_NORMAL_ROW0);
  entity.u_normalMatrix1 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_NORMAL_ROW1);
  entity.u_normalMatrix2 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_NORMAL_ROW2);

  // Extract only the fields your vertex pipeline actually targets
  entity.u_worldScale = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_WORLD_SCALE);

  vec4 entSlot9 = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_TILING);
  entity.tilingX = entSlot9.x;
  entity.tilingY = entSlot9.y;
}

void fillEntityVolumeTBO(uint entityIndex)
{
  // Each Entity spans 10 slots of vec4
  const int entBase = int(entityIndex) * ENTITY_STRIDE_VEC4;

  entity.u_worldVolume = texelFetch(u_entityFloatTBO, entBase + ENT_SLOT_WORLD_VOLUME);
}

void fillEntityFontTBO(uint entityIndex)
{
  // Each Entity spans 10 slots of vec4
  const int entBase = int(entityIndex) * ENTITY_STRIDE_VEC4;

  uvec2 fontTex = texelFetch(u_entityUintTBO, entBase + ENT_SLOT_OBJECT_AND_FLAGS).xy;

  entity.u_fontHandle = fontTex;
}

void fillEntityObjectIdTBO(uint entityIndex)
{
  // Each Entity spans 10 slots of vec4
  const int entBase = int(entityIndex) * ENTITY_STRIDE_VEC4;

  uint objectId = texelFetch(u_entityUintTBO, entBase + ENT_SLOT_OBJECT_AND_FLAGS).z;

  entity.u_objectID = objectId;
}
