layout (std430, binding = SSBO_MATERIAL_INDECES) readonly buffer MaterialIndeces {
  // NOTE KI material indeces for "per vertex" materials
  int u_materialIndeces[];
};
