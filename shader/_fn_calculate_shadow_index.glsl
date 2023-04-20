uint calculateShadowIndex(
  in vec3 viewPos)
{
  uint shadowIndex = u_shadowCount - 1;

  const float depthValue = abs(viewPos.z);

  for (int i = 0; i < u_shadowCount; i++) {
    if (depthValue <= u_shadowPlanes[i + 1].z) {
      shadowIndex = i;
      break;
    }
  }

  return shadowIndex;
}
