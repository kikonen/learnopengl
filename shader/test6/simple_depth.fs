#version 450 core

#include struct_material.glsl

#include uniform_materials.glsl

in VS_OUT {
  vec2 texCoords;
  flat int materialIndex;
} fs_in;

uniform sampler2D textures[TEX_COUNT];


void main()
{
  int matIdx = fs_in.materialIndex;
  int diffuseTexIdx = materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(textures[diffuseTexIdx], fs_in.texCoords).a;
  } else {
    alpha = materials[matIdx].diffuse.a;
  }

  if (alpha < 0.4)
    discard;

    // gl_FragDepth = gl_FragCoord.z;
}
