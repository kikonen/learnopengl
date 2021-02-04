Material resolveMaterial(int matIdx) {
  Material mat;
  mat.hasNormalMap = false;

  if (materials[matIdx].hasDiffuseTex) {
    mat.diffuse = texture(textures[matIdx].diffuse, fs_in.texCoords).rgba;
    mat.ambient = materials[matIdx].ambient;
    mat.ambient = mat.diffuse;
  } else {
    mat.diffuse = materials[matIdx].diffuse;
    mat.ambient = materials[matIdx].ambient;
  }

  if (materials[matIdx].hasEmissionTex){
    mat.emission = texture(textures[matIdx].emission, fs_in.texCoords).rgba;
  }

  if (materials[matIdx].hasSpecularTex){
    mat.specular = texture(textures[matIdx].specular, fs_in.texCoords).rgba;
  } else {
    mat.specular = materials[matIdx].specular;
  }
  mat.shininess = materials[matIdx].shininess;

  return mat;
}
