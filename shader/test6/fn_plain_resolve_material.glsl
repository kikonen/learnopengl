Material resolveMaterial(int matIdx) {
  Material mat;
  mat.hasNormalMap = false;

  Material orig = materials[matIdx];

  mat.diffuse = orig.diffuse;
  mat.ambient = orig.ambient;
  mat.emission = orig.emission;
  mat.specular = orig.specular;
  mat.shininess = orig.shininess;

  return mat;
}
