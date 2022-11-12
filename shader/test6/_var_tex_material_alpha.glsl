float alpha;
{
  uint matIdx = fs_in.materialIndex;
  int diffuseTex = u_materials[matIdx].diffuseTex;

  if (diffuseTex >= 0) {
    //alpha = texture(u_textures[diffuseTex], fs_in.texCoord).a;
    sampler2D sampler = sampler2D(u_texture_handles[diffuseTex]);
    alpha = texture(sampler, fs_in.texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }
}
