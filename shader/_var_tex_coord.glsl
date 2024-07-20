#ifdef USE_PARALLAX
  float parallaxDepth = u_materials[materialIndex].parallaxDepth;
  if (u_parallaxDepth >= 0) {
    parallaxDepth = u_parallaxDepth;
  } else {
    parallaxDepth = 0;
  }
  vec2 texCoord;

  if (parallaxDepth > 0) {
    const vec3 viewTangentDir = normalize(fs_in.viewTangentPos - fs_in.tangentPos);
    if (u_parallaxMethod == 1)
    {
      texCoord = calculateParallaxOcclusionMapping(
	materialIndex,
	fs_in.texCoord,
	viewTangentDir,
	parallaxDepth);
    }

    // https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
    if (u_parallaxMethod == 0)
    {
      texCoord = parallaxMapMarch(
	materialIndex,
	viewTangentDir,
	parallaxDepth,
	fs_in.texCoord);
    }
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
