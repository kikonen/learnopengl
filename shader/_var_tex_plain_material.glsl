{
  if (material.diffuseTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.diffuseTex]);
    material.diffuse = texture(sampler, texCoord);
  }

  if (material.opacityMapTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.opacityMapTex]);
    material.diffuse.a = texture(sampler, texCoord).r;
  }

  if (material.emissionTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.emissionTex]);
    material.emission = texture(sampler, texCoord);
    material.emission.a = 1.0;
  }

  if (material.specularTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.specularTex]);
    material.specular = vec4(texture(sampler, texCoord).xyz, material.specular.a);
  }
}
