#define _SSBO_ENTITIES
layout (std430, binding = SSBO_ENTITIES) readonly buffer EntitySSBO {
  Entity u_entities[];
};
