#define GET_INSTANCE_INDEX u_instanceIndeces[gl_BaseInstance + gl_InstanceID]
#define GET_INSTANCE u_instances[GET_INSTANCE_INDEX]

#define _SSBO_INSTANCE_INDECES
layout (std430, binding = SSBO_INSTANCE_INDECES) readonly buffer InstanceIndecesSSBO {
  uint u_instanceIndeces[];
};
