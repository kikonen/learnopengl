#include "include/tbo_offets.glsl"
#include "include/struct_instance.glsl"

#define _TBO_INSTANCES
layout(binding = UNIT_INSTANCE_FLOAT) uniform samplerBuffer u_instanceFloatTBO;
layout(binding = UNIT_INSTANCE_UINT) uniform usamplerBuffer u_instanceUintTBO;
