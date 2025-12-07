float alpha;

{
  const uint i = fs_in.materialIndex;

  float opacity = texture(sampler2D(u_materials[i].opacityMapTex), texCoord).a;

  alpha =
    (u_materials[i].diffuse *
     texture(sampler2D(u_materials[i].diffuseTex), texCoord)).a *
    opacity;
}
