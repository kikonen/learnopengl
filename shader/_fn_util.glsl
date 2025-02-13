#define DECODE_A_NORMAL(n) (n * 2.0 - 1.0)
#define DECODE_A_TANGENT(t) (t * 2.0 - 1.0)

// @see https://en.wikipedia.org/wiki/Rotation_matrix
mat4 translateMatrix(const in vec3 offset)
{
  return mat4( vec4( 1.0,      0.0,      0.0,      0.0),
	       vec4( 0.0,      1.0,      0.0,      0.0),
	       vec4( 0.0,      0.0,      1.0,      0.0),
	       vec4( offset.x, offset.y, offset.z, 1.0));
}

// @see https://en.wikipedia.org/wiki/Rotation_matrix
mat4 rotateMatrix(const in vec3 angle)
{
  const float angleX = angle.x;
  const float angleY = angle.y;
  const float angleZ = angle.z;

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

  return rotX * rotY * rotZ;
}

// @see https://en.wikipedia.org/wiki/Rotation_matrix
mat4 rotateEulerMatrix(const in vec3 euler)
{
  const float angleX = radians(euler.x);
  const float angleY = radians(euler.y);
  const float angleZ = radians(euler.z);

  return rotateMatrix(
    vec3(
      angleX,
      angleY,
      angleZ));
}

// @see https://en.wikipedia.org/wiki/Rotation_matrix
vec3 rotateEuler(const in vec3 v, const in vec3 euler)
{
  return (rotateEulerMatrix(euler) * vec4(v, 1)).xyz;
}

// Yaw/Pitch opengl
// yaw == pitch == 0 => vec3(0, 0, -1)
// => thus pointing *forwards* to scene (vec3(0, 0, 1) points to camera)
//
// @see https://stackoverflow.com/questions/1568568/how-to-convert-euler-angles-to-directional-vector
//
// @param yaw radians rotate around Y-axis
// @param pitch radians rotate around X-axis
vec3 getDir(in float yaw, in float pitch)
{
  const float x = sin(yaw);
  const float y = -(sin(pitch) * cos(yaw));
  const float z = -(cos(pitch) * cos(yaw));

  return vec3(x, y, z);
}

mat4 getViewportMatrix(vec2 size) {
  const float w = size.x;
  const float h = size.y;
  const float w2 = w / 2.0f;
  const float h2 = h / 2.0f;

  return mat4( vec4(w2,   0.0f, 0.0f, 0.0f),
	       vec4(0.0f, h2,   0.0f, 0.0f),
	       vec4(0.0f, 0.0f, 1.0f, 0.0f),
	       vec4(w2+0, h2+0, 0.0f, 1.0f));
}

// https://stackoverflow.com/questions/21576367/reading-from-a-depth-texture-in-a-fragment-shader
// @param depth [0, 1]
// @return value linearized within [near, far] plane range
float linearizeDepth(float d, float near, float far)
{
  return 2.0 * near * far / (far + near - d * (far - near));
}

// https://stackoverflow.com/questions/21576367/reading-from-a-depth-texture-in-a-fragment-shader
// @param depth [0, 1]
// @return value linearized within [near, far] plane range
float linearizeDepth2(float d, float near, float far)
{
  float z = 2.0 * d - 1.0;
  return 2.0 * near * far / (far + near - z * (far - near));
}
