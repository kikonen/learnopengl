Material material = u_materials[fs_in.materialIndex];
{
  if (material.diffuseTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.diffuseTex]);
    material.diffuse = texture(sampler, fs_in.texCoord);
  }

  if (material.emissionTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.emissionTex]);
    material.emission = texture(sampler, fs_in.texCoord);
  }

  if (material.specularTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.specularTex]);
    material.specular = texture(sampler, fs_in.texCoord);
  }
}
