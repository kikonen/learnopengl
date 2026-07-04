#define _ALPHA_RESOLVED
float alpha;
{
  float opacity = texture(sampler2D(u_materials[materialIndex].opacityMapTex), texCoord).r;

  alpha =
    (u_materials[materialIndex].diffuse.a *
     texture(sampler2D(u_materials[materialIndex].diffuseTex), texCoord)).a *
    opacity;
}
