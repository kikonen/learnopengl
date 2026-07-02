#ifdef USE_PARALLAX

if (Debug.u_parallaxEnabled && !u_forceLineMode) {
  fillMaterialParallaxTBO(materialIndex);

  uvec2 displacementTex = readMaterialDisplacementTexTBO(materialIndex);

  float parallaxDepth = Debug.u_parallaxDepth >= 0
    ? Debug.u_parallaxDepth
    : material.parallaxDepth;

#ifdef USE_DECAL
// parallaxDepth = 0.1;
#endif
// parallaxDepth = 0.0;

  if (parallaxDepth > 0) {
    const vec3 tangentDir = -normalize(tangentPos);
    if (Debug.u_parallaxMethod == 1)
    {
      texCoord = calculateParallaxOcclusionMappingTBO(
	displacementTex,
	texCoord,
	tangentDir,
	parallaxDepth);
    }

    // https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
    if (Debug.u_parallaxMethod == 0)
    {
      texCoord = parallaxMapMarchTBO(
	displacementTex,
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
