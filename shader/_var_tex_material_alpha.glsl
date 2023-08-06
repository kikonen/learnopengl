float alpha;

if (fs_in.shapeIndex > 0) {
  const int diffuseTex = u_shapes[fs_in.shapeIndex].diffuseTex;

  sampler2D sampler = sampler2D(u_texture_handles[diffuseTex]);
  alpha = texture(sampler, texCoord).a;
} else {
  const uint matIdx = fs_in.materialIndex;
  int diffuseTex = u_materials[matIdx].diffuseTex;

  if (diffuseTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[diffuseTex]);
    alpha = texture(sampler, texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }
}
