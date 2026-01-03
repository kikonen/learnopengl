#version 460 core

#include "include/ssbo_materials.glsl"

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

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  float d = textureLod(sampler2D(u_materials[materialIndex].custom1Tex), texCoord, 0).r;
  // if (d < 0.1) {
  //   discard;
  // }

  o_fragColor = vec4(d, d, d, 1);// * material.diffuse;
}
