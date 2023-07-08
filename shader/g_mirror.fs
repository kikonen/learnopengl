#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec4 glp;

  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  mat3 TBN;
} fs_in;

layout(binding = UNIT_MIRROR_REFLECTION) uniform sampler2D u_reflectionTex;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_normal_pattern.glsl

void main() {
  #include var_tex_material.glsl

  vec3 normal = fs_in.normal;

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

  if (gl_FrontFacing)
  {
    vec4 gp = fs_in.glp;
    vec2 reflectCoord = vec2(-gp.x, gp.y) / (gp.w * 2.0) + 0.5;

    vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

    vec4 mixColor = reflectColor;

    material.diffuse = mix(material.diffuse, mixColor, 0.9);
  }

  vec4 texColor = material.diffuse;

  o_fragColor = vec4(texColor.xyz, material.ambient);
  o_fragSpecular = material.specular;
  o_fragEmission = material.emission;

  o_fragPosition = fs_in.worldPos;
  o_fragNormal = normal;
}
