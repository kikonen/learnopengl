#include "include/tbo_offsets.glsl"
#include "include/struct_entity.glsl"

#define _TBO_ENTITIES
layout(binding = UNIT_ENTITY_FLOAT) uniform samplerBuffer u_entityFloatTBO;
layout(binding = UNIT_ENTITY_UINT) uniform usamplerBuffer u_entityUintTBO;
