#ifdef USE_TREE_WIND
void mod_tree_wind(
  inout vec4 pos,
  inout vec3 normal,
  inout vec3 tangent)
{
  float time = u_time;

  float x = 0.5 + pos.x * 0.5;
  float y = pos.y;

  vec2 v = vec2(pos.x, pos.y);
  float d = length(v);

  vec3 offset = vec3(
    0, //sin(u_time) * 0.2,
    cos(u_time * 3) * 0.01,
    d * 0.01 * sin(u_time * 2) * 0.02
    );

  mat4 transform = rotateMatrix(offset);

  pos = transform * pos;

  mat3 normalMatrix = mat3(transpose(inverse(transform)));
  normal =  normalMatrix * normal;

#ifdef USE_TBN
  normal = normalMatrix * tangent;
#endif
}

void mod_tree_wind(
  inout vec4 pos)
{
  vec3 dummy = vec3(0);
  mod_tree_wind(pos, dummy, dummy);
}
#endif
