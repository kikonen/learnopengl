#ifdef USE_HEIGHT_TEX
  vec2 texCoord;
  if (material.heightMapTex >= 0) {
    const vec3 tanToView = normalize(fs_in.viewTangentPos - fs_in.tangentPos);
    texCoord = calculateParallaxMapping(material, fs_in.texCoord, tanToView);
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
