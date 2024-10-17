// @see https://en.wikipedia.org/wiki/Rotation_matrix
vec3 rotateEuler(vec3 v, vec3 euler)
{
  float angleX = radians(euler.x);
  float angleY = radians(euler.y);
  float angleZ = radians(euler.z);

  mat4 rotX = mat4( vec4( 1,            0,             0.0,         0.0),
		    vec4( 0,            cos(angleX),   sin(angleX), 0.0),
		    vec4( 0.0,         -sin(angleX),   cos(angleX), 0.0),
		    vec4( 0.0,          0.0,           0.0,         1.0));

  mat4 rotY = mat4( vec4( cos(angleY),  0,            -sin(angleY), 0.0),
		    vec4( 0,            1,             0.0,         0.0),
		    vec4( sin(angleY),  0.0,           cos(angleY), 0.0),
		    vec4( 0.0,          0.0,           0.0,         1.0));

  mat4 rotZ = mat4( vec4( cos(angleZ),  sin(angleZ),   0.0,         0.0),
		    vec4( -sin(angleZ), cos(angleZ),   0.0,         0.0),
		    vec4( 0.0,          0.0,           1.0,         0.0),
		    vec4( 0.0,          0.0,           0.0,         1.0));

  return (rotX * rotY * rotZ * vec4(v, 1)).xyz;
}

// Yaw/Pitch opengl
// yaw == pitch == 0 => vec3(0, 0, -1)
// => thus pointing *forwards* to scene (vec3(0, 0, 1) points to camera)
//
// @see https://stackoverflow.com/questions/1568568/how-to-convert-euler-angles-to-directional-vector
//
// @param yaw radians rotate around Y-axis
// @param pitch radians rotate around X-axis
vec3 getDir(float yaw, float pitch)
{
  float x = sin(yaw);
  float y = -(sin(pitch) * cos(yaw));
  float z = -(cos(pitch) * cos(yaw));

  return vec3(x, y, z);
}
