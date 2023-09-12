#define materialRGB(tx) \
  if (material.tx ## Tex.x > 0) { \
    material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
  }

{
  // if (material.diffuseTex.x > 0) {
  //   sampler2D sampler = sampler2D(material.diffuseTex);
  //   material.diffuse = texture(sampler, texCoord);
  // }
  materialRGB(diffuse);

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

  if (material.metalMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.metalMapTex);
    material.metal = texture(sampler, texCoord);
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
}
