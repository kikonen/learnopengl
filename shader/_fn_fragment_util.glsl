#ifdef __FRAGMENT_SHADER__

void OIT_DISCARD(float alpha)
{
  if (alpha < OIT_MIN_BLEND_THRESHOLD || alpha >= OIT_MAX_BLEND_THRESHOLD) {
    discard;
  }
}

#endif
