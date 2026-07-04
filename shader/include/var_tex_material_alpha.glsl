#define _ALPHA_RESOLVED
#ifdef USE_ALPHA
{
  material.diffuseTexel = texture(sampler2D(u_materials[materialIndex].diffuseTex), texCoord);

  material.alpha =
    (u_materials[materialIndex].diffuse.a *
     material.diffuseTexel.a *
    texture(sampler2D(u_materials[materialIndex].opacityMapTex), texCoord).r);
}
#else
{
  material.diffuseTexel = texture(sampler2D(u_materials[materialIndex].diffuseTex), texCoord);

  material.alpha = 1.0;
}
#endif
