Material material = u_materials[fs_in.materialIndex];
{
  if (material.diffuseTex >= 0) {
    material.diffuse = texture(u_textures[material.diffuseTex], fs_in.texCoords).rgba;
    // TODO KI WHAT was going on here?!?
    //material.ambient = material.ambient;
  }

  if (material.emissionTex >= 0) {
    material.emission = texture(u_textures[material.emissionTex], fs_in.texCoords).rgba;
  }

  if (material.specularTex >= 0) {
    material.specular = texture(u_textures[material.specularTex], fs_in.texCoords).rgba;
  }
}
