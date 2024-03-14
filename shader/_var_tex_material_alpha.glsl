float alpha;

{
  const uint i = fs_in.materialIndex;

  alpha = (u_materials[i].diffuse *
	   texture(sampler2D(u_materials[i].diffuseTex), texCoord)).a *
    texture(sampler2D(u_materials[i].opacityMapTex), texCoord).r;
}
