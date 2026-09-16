#define _ALPHA_RESOLVED

#ifdef USE_DYNAMIC_TEXTURE
{
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex);
  const int dynamicLayer = int(readMaterial_dynamicTex(materialIndex));

  vec4 staticTexel  = texture(
    u_texturesSRGB,
    vec3(texCoord, float(diffuseLayer)));

  vec4 dynamicTexel = texture(
    u_texturesDynamic,
    vec3(texCoord, float(dynamicLayer)));

  float mixRatio = readMaterial_dynamicRatio(materialIndex);

  material.diffuseTexel = mix(staticTexel, dynamicTexel, mixRatio);
  material.diffuseTexel.rgb = mix(staticTexel.rgb, dynamicTexel.rgb, mixRatio);

#ifdef USE_ALPHA
#ifdef USE_BLEND
  material.diffuseTexel.a = mix(staticTexel.a, dynamicTexel.a, mixRatio);
  // material.diffuseTexel.a = min(staticTexel.a, dynamicTexel.a);
  // material.diffuseTexel.a = dynamicTexel.a;
#else
  material.diffuseTexel.a = staticTexel.a;
#endif
#endif

#ifdef USE_ALPHA
  material.alpha = u_materials[materialIndex].diffuse.a * material.diffuseTexel.a;
#else
  material.diffuseTexel.a = 1.0;
  material.alpha = 1.0;
#endif
}
#else
{
  const int diffuseLayer = int(u_materials[materialIndex].diffuseTex);

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
