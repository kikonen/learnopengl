#define _ALPHA_RESOLVED

#ifndef USE_TEXTURE_ARRAY
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
#endif

#ifdef USE_TEXTURE_ARRAY
#ifdef USE_ALPHA
{
  int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);

  // Sample unified sRGB textures array
  material.diffuseTexel = texture(u_TexturesSRGB, vec3(texCoord, float(diffuseLayer)));

  material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
}
#else
{
'  int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);

  // Sample unified sRGB textures array
  material.diffuseTexel = texture(u_TexturesSRGB, vec3(texCoord, float(diffuseLayer)));
  material.alpha = 1.0;
}
#endif
#endif
