layout (std140, binding = UBO_MATERIALS) uniform Materials {
  Material u_materials[MAT_COUNT];
};
