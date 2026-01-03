#include "include/struct_instance.glsl"

#define _SSBO_INSTANCES
layout (std430, binding = SSBO_INSTANCES) readonly buffer InstanceSSBO {
  Instance u_instances[];
};
