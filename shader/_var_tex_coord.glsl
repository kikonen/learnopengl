#ifdef USE_PARALLAX
  vec2 texCoord;
  if (material.parallaxDepth > 0) {
    const vec3 viewTangentDir = normalize(fs_in.viewTangentPos - fs_in.tangentPos);
    texCoord = calculateParallaxOcclusionMapping(material, fs_in.texCoord, viewTangentDir);
  } else {
    texCoord = fs_in.texCoord;
  }

#ifdef USE_ALPHA
  if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
    discard;
#endif

#else
  const vec2 texCoord = fs_in.texCoord;
#endif
