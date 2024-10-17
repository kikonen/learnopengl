void calculateCustomClipping(in vec4 worldPos)
{
  // yaw == pitch == 0 + plane.w == 0 => clip at origo towards scene
  float yaw = radians(-45);//45 * cos(45 - u_time * 0.5));
  float pitch = radians(0);//45 - 45 * cos(u_time));

  vec3 dir = getDir(yaw, pitch);

  // plane = vec4(-1, 1, 0, abs(0.5 + sin(u_time * 0.2) * 0.5) * 0.5);
  vec4 plane = vec4(dir, 0);

  gl_ClipDistance[1] = dot(worldPos.xyz, plane.xyz)
    + plane.w;
}
