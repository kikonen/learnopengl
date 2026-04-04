{
  const uint i = materialIndex;

  material.flags = u_materials[i].flags;

  material.diffuse = u_materials[i].diffuse;
  material.emission = u_materials[i].emission.rgb;
  material.mras = u_materials[i].mras.rgba;

  // material.reflection = u_materials[i].reflection;
  // material.refraction = u_materials[i].refraction;
  // material.refractionRatio = u_materials[i].refractionRatio;
}
