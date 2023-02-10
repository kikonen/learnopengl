Material material = u_materials[fs_in.materialIndex];
{
  if (material.diffuseTex >= 0) {
    //material.diffuse = texture(u_textures[material.diffuseTex], fs_in.texCoord);

    sampler2D sampler = sampler2D(u_texture_handles[material.diffuseTex]);
    material.diffuse = texture(sampler, fs_in.texCoord);

    // TODO KI WHAT was going on here?!?
    //material.ambient = material.ambient;
  }

  if (material.emissionTex >= 0) {
    //material.emission = texture(u_textures[material.emissionTex], fs_in.texCoord);
    sampler2D sampler = sampler2D(u_texture_handles[material.emissionTex]);
    material.emission = texture(sampler, fs_in.texCoord);
  }

  if (material.specularTex >= 0) {
    //material.specular = texture(u_textures[material.specularTex], fs_in.texCoord);
    sampler2D sampler = sampler2D(u_texture_handles[material.specularTex]);
    material.specular = texture(sampler, fs_in.texCoord);
  }
}
