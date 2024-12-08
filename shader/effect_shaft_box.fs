%#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat float radius;

  vec4 shadowPos;

  mat4 invModel;
  vec3 pos;
  vec3 cameraObjectPos;
  vec3 cameraObjectFront;
} fs_in;

LAYOUT_G_BUFFER_SAMPLERS;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

float linearizeDepth(float depth) {
  return linearizeDepth2(depth, u_nearPlane, u_farPlane);
}

float CalculateShaftBrightness(
  const in vec3 cameraObjectFront,
  const float pz,
  const vec3 vdir,
  const vec2 pixCoord,
  float t1,
  float t2)
{
  // shaftSigma = a
  // shaftRho0 = p0
  // shaftTau =
  // normalizer = 1 / D * max(p0, p1)
  float D = 1;
  float shaftSigma;
  float shaftRho0;
  float shaftTau;
  float normalizer;

  // Read z0 from the structure buffer, calculate t0, and clamp to [t0,1].
  const float depth = -textureLod(g_viewZ, pixCoord, 0).x;

  const float t0 = 1.0 + depth / dot(cameraObjectFront, vdir);
  t1 = clamp(t1, t0, 1.0);
  t2 = clamp(t2, t0, 1.0);

  // Limit to range where density is not negative.
  const float tlim = (shaftTau − pz) / vdir.z;
  if (vdir.z * shaftSigma < 0.0) {
    t1 = min(t1, tlim);
    t2 = min(t2, tlim);
  }
  else {
    t1 = max(t1, tlim);
    t2 = max(t2, tlim);
  }

  // Evaluate density integral, normalize, and square.
  float B = (shaftSigma * (pz + vdir.z * ((t1 + t2) * 0.5)) + shaftRho0)
    * (t2 − t1) * normalizer;
  return B * B * dot(vdir, vdir);
}

float CalculateBoxShaftBrightness(
  const float sx,
  const float sy,
  const in vec3 cameraObjectPos,
  const in vec3 cameraObjectFront,
  const vec3 pos,
  const vec2 pixCoord)
{
  const vec3 vdir = cameraObjectPos − pos;
  float t1 = 0.0, t2 = 1.0;

  // Find intersections with planes perpendicular to x axis.
  const float a = −pos.x / vdir.x;
  const float b = (sx − pos.x) / vdir.x;

  if (vdir.x > 0.0) {
    t1 = max(t1, a);
    t2 = min(t2, b);
  }
  else {
    t1 = max(t1, b);
    t2 = min(t2, a);
  }

  // Find intersections with planes perpendicular to y axis.
  a = −pos.y / vdir.y;
  b = (sy − pos.y) / vdir.y;

  if (vdir.y > 0.0) {
    t1 = max(t1, a);
    t2 = min(t2, b);
  }
  else {
    t1 = max(t1, b);
    t2 = min(t2, a);
  }

  return CalculateShaftBrightness(
    cameraObjectFront,
    pos.z,
    vdir,
    pixCoord,
    t1,
    t2);
}

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec2 pixCoord = gl_FragCoord.xy / u_bufferResolution;

  vec3 color = material.diffuse.rgb;
  // color = vec3(0, 0, 1);

  float sx = 1;
  float sy = 1;
  float halo = calculateBoxShaftBrightness(
    sx,
    sy,
    fs_in.cameraObjectPos,
    normalize(fs_in.cameraObjectFront),
    fs_in.pos,
    pixCoord);

  // halo *= 5;
  // halo = 1;
  float blend = halo;
  // if (false)
  {
    if (halo > 0.00001) {
      // color = vec4(0, 0, 0);
      // color.rg = fs_in.texCoord;
      // TODO KI somehow need to configure halo range in material
      blend = 0.1 + halo * 0.9;
    }
  }

  if (false)
  {
    // float dp = textureLod(g_depth, pixCoord, 0).x;
    // float depth = linearizeDepth(dp);
    // dp = 1.0 - (dp - 0.99) * 100.0;
    // color.rgb = vec3(dp);
    // if (depth < 0) {
    //   color.rgb = vec3(1, 0, 0);
    // }

    float d = textureLod(g_viewZ, pixCoord, 0).x;
    color.rgb = vec3(-d / u_farPlane);

    color.rgb = -fs_in.cameraObjectFront;
    color.rgb = fs_in.normal * 0.5 + 0.5;

    blend = 1;
    if (halo > 0.0000001) {
      color.rgb = vec3(0, 1, 0);
      blend = halo;
      blend = 0.8;
    }
  }

#ifdef USE_BLEND
  o_fragColor = vec4(color.rgb, blend);
#else
  o_fragColor = vec4(color.rgb, 1);
#endif
}
