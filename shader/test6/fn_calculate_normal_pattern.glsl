vec3 calculateNormalPattern(vec3 normal)
{
  float a = 0.25;
  float b = 50.0;
  float x = fs_in.vertexPos.x;
  float y = fs_in.vertexPos.y;
  float z = fs_in.vertexPos.z;
  vec3 N;
  N.x = normal.x + a * sin(b*x);
  N.y = normal.y + a * sin(b*y);
  N.z = normal.z + a * sin(b*z);
  return normalize(N);
}
