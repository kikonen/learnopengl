// #define materialRGBA(tx) \
//   if (material.tx ## Tex.x > 0) { \
//     material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
//   }

{
  const uint i = materialIndex;

  material.diffuse = u_materials[i].diffuse *
    texture(sampler2D(u_materials[i].diffuseTex), texCoord);

  material.diffuse.a *= texture(sampler2D(u_materials[i].opacityMapTex), texCoord).r;

  material.emission = u_materials[i].emission.rgb *
    texture(sampler2D(u_materials[i].emissionTex), texCoord + vec2(0, u_time) * -0).rgb;

  material.metal = u_materials[i].metal *
    texture(sampler2D(u_materials[i].metalTex), texCoord);

  material.reflection = u_materials[i].reflection;
  material.refraction = u_materials[i].refraction;
  material.refractionRatio = u_materials[i].refractionRatio;
}
