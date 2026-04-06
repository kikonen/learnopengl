#ifdef USE_PARALLAX
if (Debug.u_parallaxEnabled && !u_forceLineMode) {
  float parallaxDepth = Debug.u_parallaxDepth >= 0
    ? Debug.u_parallaxDepth
    : u_materials[materialIndex].parallaxDepth;

#ifdef USE_DECAL
// parallaxDepth = 0.1;
#endif
// parallaxDepth = 0.0;

  if (parallaxDepth > 0) {
    const vec3 tangentDir = -normalize(fs_in.tangentPos);
    if (Debug.u_parallaxMethod == 1)
    {
      texCoord = calculateParallaxOcclusionMapping(
	materialIndex,
	texCoord,
	tangentDir,
	parallaxDepth);
    }

    // https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
    if (Debug.u_parallaxMethod == 0)
    {
      texCoord = parallaxMapMarch(
	materialIndex,
	tangentDir,
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
}
#endif
