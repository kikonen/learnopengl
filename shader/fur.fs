#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat float furStrength;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  #include var_tex_material.glsl

  float t = material.diffuse.a;
  t *= fs_in.furStrength;
  vec4 texColor = material.diffuse * vec4(1.0, 1.0, 1.0, t);

//  if (texColor.a < 0.1)
//    discard;

  o_fragColor = texColor;
}
