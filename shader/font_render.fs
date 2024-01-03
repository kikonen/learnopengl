#version 460 core

#include struct_material.glsl
#include uniform_materials.glsl


in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} fs_in;


layout(binding = UNIT_FONT_ATLAS) uniform sampler2D u_fontAtlas;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

void main()
{
  material = u_materials[fs_in.materialIndex];

#include var_tex_material.glsl

  float a = texture(u_fontAtlas, fs_in.texCoord).r;

  if (a < 0.0001) {
    discard;
  }

  vec4 texColor = vec4(material.diffuse.xyz, a);

  o_fragColor = texColor;
}
