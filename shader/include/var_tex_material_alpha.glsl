#define _ALPHA_RESOLVED

#ifndef USE_TEXTURE_ARRAY
#ifdef USE_ALPHA
{
  material.diffuseTexel = texture(
    sampler2D(u_materials[materialIndex].diffuseTex),
    texCoord);

  material.alpha =
    u_materials[materialIndex].diffuse.a *
     material.diffuseTexel.a;
}
#else
{
  material.diffuseTexel = texture(
    sampler2D(u_materials[materialIndex].diffuseTex),
    texCoord);

  material.alpha = 1.0;
}
#endif
#endif

#ifdef USE_TEXTURE_ARRAY
#ifdef USE_DYNAMIC_TEXTURE
{
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);
  const int dynamicLayer = int(readMaterial_dynamicTex(materialIndex).x);

  vec4 staticTexel  = texture(
    u_texturesSRGB,
    vec3(texCoord, float(diffuseLayer)));

  vec4 dynamicTexel = texture(
    u_texturesDynamic,
    vec3(texCoord, float(dynamicLayer)));

  float mixRatio = u_materials[materialIndex].dynamicRatio;
  material.diffuseTexel = mix(staticTexel, dynamicTexel, mixRatio);

#ifdef USE_ALPHA
  material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
#else
  material.alpha = 1.0;
#endif
}
#else
{
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex.x);

  material.diffuseTexel = texture(
    u_texturesSRGB,
    vec3(texCoord, float(diffuseLayer)));

#ifdef USE_ALPHA
  material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
#else
  material.alpha = 1.0;
#endif
}
#endif
#endif
