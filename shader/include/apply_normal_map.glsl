#ifdef USE_NORMAL_TEX
{
  if (Debug.u_normalMapEnabled && u_materials[materialIndex].normalMapTex.x > 0) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    normal = texture(sampler, texCoord).rgb * 2.0 - 1.0;
    normal = normalize(fs_in.tbn * normal);
  }
}
#endif
