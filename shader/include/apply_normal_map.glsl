// TODO KI KTX_TTF_BC5_RG support
// vec3 normal = texture(normalMap, uv).rgb;
// // BC5 only stores RG, B will be 0
// normal.xy = normal.rg * 2.0 - 1.0;
// normal.z = sqrt(max(1.0 - dot(normal.xy, normal.xy), 0.0));

#ifndef USE_TEXTURE_ARRAY
#ifdef USE_NORMAL_TEX
{
  if (Debug.u_normalMapEnabled) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    normal = texture(sampler, texCoord).rgb * 2.0 - 1.0;
    normal = normalize(tbn * normal);
  }
}
#endif
#endif

#ifdef USE_TEXTURE_ARRAY
#ifdef USE_NORMAL_TEX
{
  if (Debug.u_normalMapEnabled) {
    int normalLayer = int(u_materials[materialIndex].normalMapTex.x);

    // Only apply normal mapping if a valid texture is explicitly bound
    // Layer 0 is reserved for NULL/None, which skips texture sampling
    if (normalLayer > 0) {
      vec3 normalTexel = texture(u_texturesNormal, vec3(texCoord, float(normalLayer))).rgb * 2.0 - 1.0;

      // Override the geometry normal only with the transformed tangent space vector
      normal = normalize(tbn * normalTexel);
    }
  }
}
#endif
#endif
