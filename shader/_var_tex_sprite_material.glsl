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

if (fs_in.shapeIndex > 0) {
  Shape shape = u_shapes[fs_in.shapeIndex];

  {
    sampler2D sampler = sampler2D(u_texture_handles[shape.diffuseTex]);
    material.diffuse = texture(sampler, texCoord);
  }

  if (shape.emissionTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[shape.emissionTex]);
    material.emission = texture(sampler, texCoord);
    material.emission.a = 1.0;
  }

  if (shape.specularTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[shape.specularTex]);
    material.specular = vec4(texture(sampler, texCoord).xyz, material.specular.a);
  }
}
