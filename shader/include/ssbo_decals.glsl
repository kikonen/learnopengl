#include "include/struct_decal.glsl"

#define _SSBO_DECALS
layout (std430, binding = SSBO_DECALS) readonly buffer DecalSSBO {
  Decal u_decals[];
};
