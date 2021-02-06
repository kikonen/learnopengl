#version 330 core

#include struct_material.glsl

struct Texture {
  sampler2D diffuse;
};

#include uniform_materials.glsl

in VS_OUT {
  vec2 texCoords;
  flat int materialIndex;
} fs_in;

uniform Texture textures[MAT_COUNT];

void main()
{
  int matIdx = fs_in.materialIndex;

  float alpha;
  if (materials[matIdx].hasDiffuseTex) {
    alpha = texture(textures[matIdx].diffuse, fs_in.texCoords).a;
  } else {
    alpha = materials[matIdx].diffuse.a;
  }

  if (alpha < 0.4)
    discard;

    // gl_FragDepth = gl_FragCoord.z;
}
