{
  if (material.diffuseTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.diffuseTex]);
    material.diffuse = texture(sampler, texCoord);
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
