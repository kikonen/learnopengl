#ifdef USE_FLAG_WAVE
void mod_flag_wave(
  inout vec4 pos,
  inout vec3 normal,
  inout vec3 tangent)
{
  float time = 20;

  float x = 0.5 + pos.x * 0.5;
  float y = pos.y;

  pos.y += x * (sin(x + x) * sin(time * 1.5) * 0.4);
  pos.x += x * (sin(x * x) * cos(time * 1.7) * 0.4);
  pos.z += x * (cos(x * y) + sin(time) * 0.4);

  normal = rotateEuler(normal, vec3(0, -30, 0));

#ifdef USE_TBN
#endif
}

void mod_flag_wave(
  inout vec4 pos)
{
  float time = 20;

  float x = 0.5 + pos.x * 0.5;
  float y = pos.y;

  pos.y += x * (sin(x + x) * sin(time * 1.5) * 0.4);
  pos.x += x * (sin(x * x) * cos(time * 1.7) * 0.4);
  pos.z += x * (cos(x * y) + sin(time) * 0.4);
}
#endif
