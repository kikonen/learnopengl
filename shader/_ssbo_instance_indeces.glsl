#include struct_instance.glsl

#define _SSBO_INSTANCE_INDECES
layout (std430, binding = SSBO_INSTANCE_INDECES) readonly buffer InstanceIndeces {
  Instance u_instances[];
};
