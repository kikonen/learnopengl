#version 460 core

#include "include/tbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_lights.glsl"

in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;

  #include "include/apply_parallax_tbo.glsl"
  fillMaterialTBO(materialIndex, texCoord);

  uvec2 customTex = readMaterialCustom1TexTBO(materialIndex);

  float d = textureLod(sampler2D(customTex), texCoord, 0).r;
  // if (d < 0.1) {
  //   discard;
  // }

  o_fragColor = vec4(d, d, d, 1);// * material.diffuse;
}
