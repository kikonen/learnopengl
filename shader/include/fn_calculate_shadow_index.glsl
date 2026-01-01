uint calculateShadowIndex(
  in vec3 viewPos)
{
  const float depthValue = abs(viewPos.z);

  if (depthValue <= u_shadowCascade_0) {
    return 0;
  } else if (depthValue <= u_shadowCascade_1) {
    return 1;
  } else if (depthValue <= u_shadowCascade_2) {
    return 2;
  } else if (depthValue <= u_shadowCascade_3) {
    return 3;
  }

  return 0;//u_shadowCount;
}
