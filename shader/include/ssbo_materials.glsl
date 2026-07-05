#include "include/struct_material.glsl"
#include "include/struct_resolved_material.glsl"

#define _SSBO_MATERIALS
layout (std430, binding = SSBO_MATERIALS) readonly buffer MaterialSSBO {
  Material u_materials[];
};


float readMaterialParallaxDepth(uint i) { return u_materials[i].parallaxDepth; }
float readMaterialLayersDepth(uint i)  { return u_materials[i].layersDepth; }
float readMaterialPointSize(uint i)    { return u_materials[i].pointSize; }
