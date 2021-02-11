int matIdx = fs_in.materialIndex;
Material material = materials[matIdx];
{
  if (material.hasDiffuseTex) {
    material.diffuse = texture(textures[matIdx].diffuse, fs_in.texCoords).rgba;
    material.ambient = material.ambient;
  }

  if (material.hasEmissionTex) {
    material.emission = texture(textures[matIdx].emission, fs_in.texCoords).rgba;
  }

  if (material.hasSpecularTex) {
    material.specular = texture(textures[matIdx].specular, fs_in.texCoords).rgba;
  }
}
