#ifdef USE_PARALLAX
  float parallaxDepth = u_materials[materialIndex].parallaxDepth;
  if (Debug.u_parallaxDepth >= 0) {
    parallaxDepth = Debug.u_parallaxDepth;
  }
  if (u_forceLineMode) {
    parallaxDepth = 0;
  }

#ifdef USE_DECAL
// parallaxDepth = 0.1;
#endif
// parallaxDepth = 0.0;

  if (parallaxDepth > 0) {
    const vec3 viewTangentDir = normalize(fs_in.viewTangentPos - fs_in.tangentPos);
    if (Debug.u_parallaxMethod == 1)
    {
      texCoord = calculateParallaxOcclusionMapping(
	materialIndex,
	texCoord,
	viewTangentDir,
	parallaxDepth);
    }

    // https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
    if (Debug.u_parallaxMethod == 0)
    {
      texCoord = parallaxMapMarch(
	materialIndex,
	viewTangentDir,
	parallaxDepth,
	texCoord);
    }
  }

// NOTE KI with texture tiling in material, texCoords *CAN*
// always be beyond [0, 1] range; thus this check is invalid
// #ifdef USE_ALPHA
//   if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
//     discard;
// #endif
#endif
