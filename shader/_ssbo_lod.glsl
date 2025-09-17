#include struct_lod.glsl

#define _SSBO_LODS
layout (std430, binding = SSBO_LODS) readonly buffer LodSSBO {
  Lod u_lods[];
};
