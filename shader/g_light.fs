#version 460 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

in VS_OUT {
  flat uint materialIndex;

  vec2 texCoord;
  vec3 worldPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec4 o_fragAmbient;
layout (location = 4) out vec3 o_fragPosition;
layout (location = 5) out vec3 o_fragNormal;
layout (location = 6) out uint o_fragMaterial;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

void main() {
  Material material = u_materials[fs_in.materialIndex];

  // combined
  vec4 texColor = material.diffuse;

  o_fragMaterial = fs_in.materialIndex;
  o_fragColor = texColor;
  o_fragSpecular = material.specular;
  o_fragSpecular.a = material.shininess;
  o_fragEmission = texColor;
  o_fragAmbient = texColor;

  o_fragPosition = fs_in.worldPos;
  o_fragNormal = fs_in.normal;
}
