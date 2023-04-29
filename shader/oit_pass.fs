#version 460 core

#include struct_material.glsl

#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} fs_in;


layout (location = 0) out vec4 o_accum;
layout (location = 1) out float o_reveal;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

void main()
{
  #include var_tex_material.glsl
  vec4 color = material.diffuse;

  const float alpha = color.a;

  if (alpha < 0.01 || alpha >= 0.95)
    discard;

  float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  o_accum = vec4(color.rgb * alpha, alpha) * weight;

//  o_accum = vec4(1, 1, 0, 1);
//  o_reveal = 0.5;
}
