Material resolveMaterial(int matIdx) {
  Material mat;
  mat.hasNormalMap = false;

  Material orig = materials[matIdx];

  if (orig.hasDiffuseTex) {
    mat.diffuse = texture(textures[matIdx].diffuse, fs_in.texCoords).rgba;
    mat.ambient = orig.ambient;
  } else {
    mat.diffuse = orig.diffuse;
    mat.ambient = orig.ambient;
  }

  if (orig.hasEmissionTex){
    mat.emission = texture(textures[matIdx].emission, fs_in.texCoords).rgba;
  } else {
    mat.emission = orig.emission;
  }

  if (orig.hasSpecularTex){
    mat.specular = texture(textures[matIdx].specular, fs_in.texCoords).rgba;
  } else {
    mat.specular = orig.specular;
  }
  mat.shininess = orig.shininess;

  return mat;
}
