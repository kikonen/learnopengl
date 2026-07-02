#include "include/tbo_offsets.glsl"
#include "include/struct_material.glsl"
#include "include/struct_resolved_material.glsl"

#define _TBO_MATERIALS
layout(binding = UNIT_MATERIAL_FLOAT) uniform samplerBuffer u_materialFloatTBO;
layout(binding = UNIT_MATERIAL_UINT) uniform usamplerBuffer u_materialUintTBO;
