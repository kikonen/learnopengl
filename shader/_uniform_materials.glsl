layout (std430, binding = SSBO_MATERIALS) readonly buffer MaterialSSBO {
  Material u_materials[];
};
