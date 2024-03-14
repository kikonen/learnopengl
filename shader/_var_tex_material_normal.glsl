#ifdef USE_NORMAL_TEX
vec3 normal;
{
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    const vec3 N = normalize(fs_in.normal);
    const vec3 T = normalize(fs_in.tangent);
    const vec3 B = cross(N, T);
    const mat3 TBN = mat3(T, B, N);

    normal = texture(sampler, texCoord).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);
  } else {
    // NOTE KI model *can* have multiple materials; some with normalTex
    normal = normalize(fs_in.normal);
  }
}
#else
// NOTE KI interpolation from vs to fs denormalizes normal
vec3 normal = normalize(fs_in.normal);
#endif
