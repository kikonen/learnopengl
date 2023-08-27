float alpha;

if (fs_in.shapeIndex > 0) {
  const uvec2 diffuseTex = u_shapes[fs_in.shapeIndex].diffuseTex;

  sampler2D sampler = sampler2D(diffuseTex);
  alpha = texture(sampler, texCoord).a;
} else {
  const uint matIdx = fs_in.materialIndex;
  uvec2 diffuseTex = u_materials[matIdx].diffuseTex;
  uvec2 opacityMapTex = u_materials[matIdx].opacityMapTex;

  if (diffuseTex.x > 0) {
    sampler2D sampler = sampler2D(diffuseTex);
    alpha = texture(sampler, texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }

  if (opacityMapTex.x > 0) {
    sampler2D sampler = sampler2D(opacityMapTex);
    alpha = texture(sampler, texCoord).r;
  }
}
