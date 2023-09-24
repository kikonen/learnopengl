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

  if (material.metalMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.metalMapTex);
    material.metal = texture(sampler, texCoord);
  }
}
