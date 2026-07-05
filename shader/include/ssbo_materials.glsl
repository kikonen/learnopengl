#include "include/struct_material.glsl"
#include "include/struct_resolved_material.glsl"

#define _SSBO_MATERIALS
layout (std430, binding = SSBO_MATERIALS_MAIN) readonly buffer MaterialMainSSBO {
  MaterialMain u_materials[];
};

layout (std430, binding = SSBO_MATERIALS_CUSTOM) readonly buffer MaterialCustomSSBO {
  MaterialCustom u_materialsCustom[];
};
layout (std430, binding = SSBO_MATERIALS_COLD) readonly buffer MaterialColdSSBO {
  MaterialCold u_materialsCold[];
};

float readMaterial_parallaxDepth(uint i) { return u_materials[i].parallaxDepth; }
int readMaterial_layers(uint i) {
  return int(u_materialsCold[i].layers);
}
float readMaterial_layersDepth(uint i)   { return u_materialsCold[i].layersDepth; }
float readMaterial_pointSize(uint i)     { return u_materialsCold[i].pointSize; }

uint readMaterial_packedSprites(uint i)     { return u_materialsCold[i].packedSprites; }

float readMaterial_reflection(uint i)     { return u_materialsCold[i].reflection; }
float readMaterial_refraction(uint i)     { return u_materialsCold[i].refraction; }
float readMaterial_refractionRatio(uint i)     { return u_materialsCold[i].refractionRatio; }

uvec2 readMaterial_displacementMapTex(uint i)     { return u_materialsCustom[i].displacementMapTex; }
uvec2 readMaterial_dudvMapTex(uint i)     { return u_materialsCustom[i].dudvMapTex; }
uvec2 readMaterial_noiseMapTex(uint i)     { return u_materialsCustom[i].noiseMapTex; }
uvec2 readMaterial_noise2MapTex(uint i)     { return u_materialsCustom[i].noise2MapTex; }
uvec2 readMaterial_custom1Tex(uint i)     { return u_materialsCustom[i].custom1Tex; }
