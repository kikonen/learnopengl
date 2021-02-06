Material material;
{
  material.hasDiffuseTex = materials[matIdx].hasDiffuseTex;
  material.hasEmissionTex = materials[matIdx].hasEmissionTex;
  material.hasSpecularTex = materials[matIdx].hasSpecularTex;
  material.hasNormalMap = materials[matIdx].hasNormalMap;

  if (material.hasDiffuseTex) {
    material.diffuse = texture(textures[matIdx].diffuse, fs_in.texCoords).rgba;
    material.ambient = materials[matIdx].ambient;
    material.ambient = material.diffuse;
  } else {
    material.diffuse = materials[matIdx].diffuse;
    material.ambient = materials[matIdx].ambient;
  }

  if (material.hasEmissionTex) {
    material.emission = texture(textures[matIdx].emission, fs_in.texCoords).rgba;
  }

  if (material.hasSpecularTex) {
    material.specular = texture(textures[matIdx].specular, fs_in.texCoords).rgba;
  } else {
    material.specular = materials[matIdx].specular;
  }
  material.shininess = materials[matIdx].shininess;
}
