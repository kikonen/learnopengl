#ifdef USE_FLAG_WAVE
void mod_flag_wave(
  inout vec4 pos,
  inout vec3 normal,
  inout vec3 tangent)
{
  float x = 0.5 + pos.x * 0.5;
  float y = pos.y;

  vec3 offset = vec3(
    2.2 * x * (sin(x + x) * sin(u_time * 1.5) * 0.4),
    2.2 * x * (sin(x * x) * cos(u_time * 1.7) * 0.4),
    0.2 * x * (cos(x * y) + sin(u_time) * 0.4));

  mat4 transform = rotateMatrix(offset);

  pos = transform * pos;

  // NOTE KI since just rotate is used no need for inverse
  // => if doing scale/translate then needed
  // mat3 normalMatrix = mat3(transpose(inverse(transform)));
  mat3 normalMatrix = mat3(transform);
  normal =  normalMatrix * normal;

#ifdef USE_TBN
  tangent = normalMatrix * tangent;
#endif
}

void mod_flag_wave(
  inout vec4 pos)
{
  vec3 dummy = vec3(0);
  mod_flag_wave(pos, dummy, dummy);
}
#endif
