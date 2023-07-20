#ifdef USE_HEIGHT_TEX
vec2 calculateParallaxMapping(
  in Material material,
  vec2 texCoord,
  vec3 toView)
{
  sampler2D sampler = sampler2D(u_texture_handles[material.heightMapTex]);
  float height =  texture(sampler, texCoord).r;
  vec2 p = toView.xy / toView.z * (height * material.depth);
  return texCoord - p;
}
#endif
