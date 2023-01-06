layout (std430, binding = SSBO_MESHES) readonly buffer MeshSSBO {
  Mesh u_meshes[];
};
