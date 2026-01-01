#ifdef USE_NORMAL_PATTERN
vec3 calculateNormalPattern(
  in vec3 vertexPos,
  in vec3 normal)
{
  float a = 0.25;
  float b = 50.0;
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  vec3 N;
  N.x = normal.x + a * sin(b*x);
  N.y = normal.y + a * sin(b*y);
  N.z = normal.z + a * sin(b*z);
  return normalize(N);
}
#endif
