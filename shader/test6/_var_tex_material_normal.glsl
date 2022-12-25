#ifdef USE_NORMAL_TEX
  //vec3 normal = texture(u_textures[material.normalMapTex], fs_in.texCoord).rgb;
  vec3 normal;
  if (material.normalMapTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.normalMapTex]);
    normal = texture(sampler, fs_in.texCoord).rgb;

    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
  } else {
    normal = fs_in.normal;
  }
#else
  vec3 normal = fs_in.normal;
#endif
