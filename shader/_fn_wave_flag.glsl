#ifdef USE_FLAG_WAVE
void wave_flag(inout vec4 pos)
{
  float x = 0.5 + pos.x * 0.5;
  float y = pos.y;

  pos.y += x * (sin(x + x) * sin(u_time * 1.5) * 0.4);
  pos.x += x * (sin(x * x) * cos(u_time * 1.7) * 0.4);
  pos.z += x * (cos(x * y) + sin(u_time) * 0.4);
}
#endif
