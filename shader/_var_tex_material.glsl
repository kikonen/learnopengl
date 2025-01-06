// #define materialRGBA(tx) \
//   if (material.tx ## Tex.x > 0) { \
//     material.tx = texture(sampler2D(material.tx ## Tex), texCoord); \
//   }

{
  const uint i = materialIndex;

  material.diffuse = u_materials[i].diffuse *
    texture(sampler2D(u_materials[i].diffuseTex), texCoord);

  material.diffuse.a *= texture(sampler2D(u_materials[i].opacityMapTex), texCoord).r;

  // NOTE KI discard any trash, which is possibly hidden into emission tex with alpha
  // thus (0, 0, 0) == (r, g, b, 0)
  vec4 emission = texture(
    sampler2D(u_materials[i].emissionTex),
    texCoord + vec2(0, u_time) * -0);

  material.emission = u_materials[i].emission.rgb *
    emission.rgb * emission.a;

  material.metal = u_materials[i].metal *
    texture(sampler2D(u_materials[i].metalTex), texCoord);

  material.reflection = u_materials[i].reflection;
  material.refraction = u_materials[i].refraction;
  material.refractionRatio = u_materials[i].refractionRatio;
}
