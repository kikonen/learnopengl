{
  if (material.diffuseTex.x > 0) {
    sampler2D sampler = sampler2D(material.diffuseTex);
    material.diffuse = texture(sampler, texCoord);
  }

  if (material.opacityMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.opacityMapTex);
    material.diffuse.a = texture(sampler, texCoord).r;
  }

  if (material.emissionTex.x > 0) {
    sampler2D sampler = sampler2D(material.emissionTex);
    material.emission = texture(sampler, texCoord);
    material.emission.a = 1.0;
  }

  if (material.specularTex.x > 0) {
    sampler2D sampler = sampler2D(material.specularTex);
    material.specular = vec4(texture(sampler, texCoord).xyz, material.specular.a);
  }
}

if (fs_in.shapeIndex > 0) {
  Shape shape = u_shapes[fs_in.shapeIndex];

  {
    sampler2D sampler = sampler2D(shape.diffuseTex);
    material.diffuse = texture(sampler, texCoord);
  }

  if (shape.emissionTex.x > 0) {
    sampler2D sampler = sampler2D(shape.emissionTex);
    material.emission = texture(sampler, texCoord);
    material.emission.a = 1.0;
  }

  if (shape.specularTex.x > 0) {
    sampler2D sampler = sampler2D(shape.specularTex);
    material.specular = vec4(texture(sampler, texCoord).xyz, material.specular.a);
  }

  vec4 metal = vec4(0);

  if (material.metalnessMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.metalnessMapTex);
    metal.r = texture(sampler, texCoord).r;
  }
  if (material.roughnessMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.roughnessMapTex);
    metal.g = texture(sampler, texCoord).r;
  }
  if (material.displacementMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.displacementMapTex);
    metal.b = texture(sampler, texCoord).r;
  }
  if (material.occlusionMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.occlusionMapTex);
    metal.b = texture(sampler, texCoord).r;
  }

  material.metal = metal;
}
