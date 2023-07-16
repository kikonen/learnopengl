#ifdef USE_NORMAL_TEX
  vec3 normal;
  if (material.normalMapTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.normalMapTex]);
    normal = texture(sampler, fs_in.texCoord).rgb;

    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
  } else {
    // NOTE KI model *can* have multiple materials; some with normalTex
    normal = normalize(fs_in.normal);
  }
#else
  // NOTE KI interpolation from vs to fs denormalizes normal
  vec3 normal = normalize(fs_in.normal);
#endif
