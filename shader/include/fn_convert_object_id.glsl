vec4 convertObjectID(uint objectID) {
  uint r = (objectID & 0x000000FF) >> 0;
  uint g = (objectID & 0x0000FF00) >> 8;
  uint b = (objectID & 0x00FF0000) >> 16;
  uint a = (objectID & 0xFF000000) >> 24;

  vec4 res;
  res.r = float(r) / 255.0;
  res.g = float(g) / 255.0;
  res.b = float(b) / 255.0;
  res.a = float(a) / 255.0;

  return res;
}
