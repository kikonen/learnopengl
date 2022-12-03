//layout (std140, binding = UBO_MATERIALS) uniform Materials {
//  Material u_materials[MAT_COUNT];
//};

layout (std430, binding = SSBO_MATERIALS) readonly buffer Materials {
  Material u_materials[];
};
