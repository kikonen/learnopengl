// #define materialRGBA(tx) \
//   if (material.tx ## Tex.x > 0) { \
//     material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
//   }

{
  material.diffuse *= texture(sampler2D(material.diffuseTex), texCoord);
  material.diffuse.a *= texture(sampler2D(material.opacityMapTex), texCoord).r;

  material.emission *= texture(sampler2D(material.emissionTex), texCoord);
  material.metal *= texture(sampler2D(material.metalTex), texCoord);
}
