// TODO KI KTX_TTF_BC5_RG support
// vec3 normal = texture(normalMap, uv).rgb;
// // BC5 only stores RG, B will be 0
// normal.xy = normal.rg * 2.0 - 1.0;
// normal.z = sqrt(max(1.0 - dot(normal.xy, normal.xy), 0.0));

#ifdef USE_NORMAL_TEX
{
  if (Debug.u_normalMapEnabled && u_materials[materialIndex].normalMapTex.x > 0) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    normal = texture(sampler, texCoord).rgb * 2.0 - 1.0;
    normal = normalize(fs_in.tbn * normal);
  }
}
#endif
