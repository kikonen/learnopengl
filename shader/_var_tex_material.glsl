#define materialRGBA(tx) \
  if (material.tx ## Tex.x > 0) { \
    material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
  }

{
  materialRGBA(diffuse);
  materialRGBA(emission);
  materialRGBA(metal);

  if (material.opacityMapTex.x > 0) {
    sampler2D sampler = sampler2D(material.opacityMapTex);
    material.diffuse.a = texture(sampler, texCoord).r;
  }
}
