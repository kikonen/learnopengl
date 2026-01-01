/**
 * Part 1 Challenges
 * - Make the circle yellow
 * - Make the circle smaller by decreasing its radius
 * - Make the circle smaller by moving the camera back
 * - Make the size of the circle oscillate using the sin() function and the iTime
 *   uniform provided by shadertoy
 */

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.00001;

float sdPlane( vec3 p )
{
  return p.y;
}

float sdSphere(vec3 p, float s)
{
  return length(p) - s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 opU(vec2 d1, vec2 d2)
{
  return (d1.x < d2.x) ? d1 : d2;
}

/**
 * Signed distance function describing the scene.
 *
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
vec2 sceneSDF(vec3 p) {
  vec2 res = vec2(100, 0.0);

  vec2 o1 = vec2(
    sdSphere(
      p - vec3(-0.25 + sin(iTime * 0.7) * 0.25, 0.25 + cos(iTime * 0.7) * 0.25, 0.2),
      0.2),
    4);

  vec3 pp = p - vec3(0.25 + sin(iTime * 0.6) * 0.25, 0.25 + cos(iTime * 0.6) * 0.25, 0.3);
  pp.x += sin(iTime) * 0.5;
  vec2 o2 = vec2(
    sdBox(
      pp,
      vec3(0.2)),
    5);

  vec2 o3 = vec2(
    sdSphere(
      p - vec3(-0.25 + sin(iTime * 0.5) * 0.25, -0.25 + cos(iTime * 0.5) * 0.25, 0.4),
      0.2),
    6);

  vec2 o4 = vec2(
    sdSphere(
      p - vec3(0.25 + sin(iTime * 0.4) * 0.25, -0.25 + cos(iTime * 0.4) * 0.25, 0.5),
      0.2),
    7);

  vec2 o5 = vec2(
    sdPlane(
      p - vec3(0, -0.4, 0)),
    8);

  res = opU(res, o1);
  res = opU(res, o2);
  res = opU(res, o3);
  res = opU(res, o4);
  res = opU(res, o5);

  return res;
}

/**
 * Return the shortest distance from the eyepoint to the scene surface along
 * the marching direction. If no part of the surface is found between start and end,
 * return end.
 *
 * eye: the eye point, acting as the origin of the ray
 * marchingDirection: the normalized direction to march in
 * start: the starting distance away from the eye
 * end: the max distance away from the ey to march before giving up
 */
vec2 shortestDistanceToSurface(
  vec3 eye,
  vec3 marchingDirection,
  float start,
  float end)
{
  float depth = start;

  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    vec2 dist = sceneSDF(eye + depth * marchingDirection);

    if (dist.x < EPSILON) {
      return vec2(depth, dist.y);
    }

    depth += dist.x;

    if (depth >= end) {
      return vec2(end, 0);
    }
  }
  return vec2(end, 0);
}


/**
 * Return the normalized direction to march in from the eye point for a single pixel.
 *
 * fieldOfView: vertical field of view in degrees
 * size: resolution of the output image
 * fragCoord: the x,y coordinate of the pixel in the output image
 */
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
  vec2 xy = fragCoord - size / 2.0;
  float z = size.y / tan(radians(fieldOfView) / 2.0);
  return normalize(vec3(xy, -z));
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec3 dir = rayDirection(45.0, iResolution.xy, fragCoord);
  vec3 eye = vec3(
    0, //0.5 + sin(iTime) * 0.2,
    0, //0.5 + cos(iTime) * 0.2,
    5.0);
  vec2 dist = shortestDistanceToSurface(eye, dir, MIN_DIST, MAX_DIST);

  if (dist.x > MAX_DIST - EPSILON) {
    // fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    fragColor = vec4(1.0, 0.0, 0.0, 0.2);
    return;
  }

  float alpha = 1;
  vec3 color = vec3(0);
  if (dist.y <= 0) {
  } else if (dist.y <= 4) {
    color = vec3(1, 0, 0);
  } else if (dist.y <= 5) {
    color = vec3(0, 1, 0);
  } else if (dist.y <= 6) {
    color = vec3(0, 0, 1);
  } else if (dist.y <= 7) {
    color = vec3(1, 1, 0);
  } else if (dist.y <= 8) {
    color = vec3(0.01, 0.1, 0.6);
  }

//  alpha = 0.5 + (0.5 + sin(iTime * 0.5) * 0.5) * 0.5;
  fragColor = vec4(color, alpha);

  // fragColor = vec4(1, 0, 0, 1);
  // fragColor.rgb *= iMaterial.diffuse.rgb;
}
