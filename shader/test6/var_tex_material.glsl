int matIdx = fs_in.materialIndex;
Material material = materials[matIdx];
{
  if (material.diffuseTex >= 0) {
    material.diffuse = texture(textures[material.diffuseTex], fs_in.texCoords).rgba;
    material.ambient = material.ambient;
  }

  if (material.emissionTex >= 0) {
    material.emission = texture(textures[material.emissionTex], fs_in.texCoords).rgba;
  }

  if (material.specularTex >= 0) {
    material.specular = texture(textures[material.specularTex], fs_in.texCoords).rgba;
  }
}
