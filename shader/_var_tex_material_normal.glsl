#ifdef USE_NORMAL_TEX
vec3 normal;
{
  if (Debug.u_normalMapEnabled && u_materials[materialIndex].normalMapTex.x > 0) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    normal = texture(sampler, texCoord).rgb * 2.0 - 1.0;
    normal = normalize(fs_in.tbn * normal);
  } else {
    // NOTE KI model *can* have multiple materials; some with normalTex
    normal = normalize(fs_in.normal);
  }
}
#else
// NOTE KI interpolation from vs to fs denormalizes normal
vec3 normal = normalize(fs_in.normal);
#endif
