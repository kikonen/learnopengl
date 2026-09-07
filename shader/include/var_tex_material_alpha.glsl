#define _ALPHA_RESOLVED

#ifndef USE_TEXTURE_ARRAY
#ifdef USE_ALPHA
{
  material.diffuseTexel = texture(sampler2D(u_materials[materialIndex].diffuseTex), texCoord);

  material.alpha =
    u_materials[materialIndex].diffuse.a *
     material.diffuseTexel.a;
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
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);

  // Sample unified sRGB textures array
  material.diffuseTexel = texture(u_texturesSRGB, vec3(texCoord, float(diffuseLayer)));

  material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
}
#else
{
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);

  // Sample unified sRGB textures array
  material.diffuseTexel = texture(u_texturesSRGB, vec3(texCoord, float(diffuseLayer)));
  material.alpha = 1.0;
}
#endif
#endif
