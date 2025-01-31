float alpha;

{
  const uint i = fs_in.materialIndex;

  float opacity =
    u_materials[i].mrao.a *
    texture(sampler2D(u_materials[i].mraoMapTex), texCoord).a;

  alpha =
    (u_materials[i].diffuse *
     texture(sampler2D(u_materials[i].diffuseTex), texCoord)).a *
    opacity;
}
