#version 460 core

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

layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
// layout(binding = UNIT_G_VIEW_POSITION) uniform sampler2D g_viewPosition;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;
// layout(binding = UNIT_G_VIEW_Z) uniform sampler2D g_viewZ;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_gbuffer_decode.glsl

ResolvedMaterial material;

float linearizeDepth(float depth) {
  return linearizeDepth2(depth, u_nearPlane, u_farPlane);
}

// R = halo radius
// R2 = R^2
// recipR2 = 1 / R^2
// recip3R2 = 1 / 3 * R^2
// normalizer = 3 / 4 * R
float calculateHaloBrightness(
  const in float radius,
  const in vec3 cameraObjectPos,
  const in vec3 cameraObjectFront,
  const in vec3 pos,
  const in vec2 pixCoord)
{
  const float R = radius; // Radius of the volumetric point light halo.
  const float R2 = R * R;
  const float recipR2 = 1.f / R2;
  const float recip3R2 = 1.f / (3.f * R2);
  const float normalizer = 3.f / (4.f * R);

  const vec3 vdir = (cameraObjectPos - pos);

  const float v2 = dot(vdir, vdir);
  const float p2 = dot(pos, pos);
  const float pv = -dot(pos, vdir);
  const float m = sqrt(max(pv * pv - v2 * (p2 - R2), 0.0));

  // Read z0 from the structure buffer.
  // const vec2 depth = vec2(1, 1); //texture(structureBuffer, pixCoord).zw;
  float d;
  {
    // float depth = textureLod(g_depth, pixCoord, 0).x * 2.0 - 1.0;

    // vec4 clip = vec4(pixCoord.x * 2.0 - 1.0, pixCoord.y * 2.0 - 1.0, depth, 1.0);
    // vec4 viewW  = u_invProjectionMatrix * clip;
    // vec3 viewPos  = viewW.xyz / viewW.w;
    // d = viewPos.z;

    // d = -textureLod(g_viewPosition, pixCoord, 0).z;
    // d = -textureLod(g_viewZ, pixCoord, 0).x;
    d = -getViewPosFromGBuffer(pixCoord).z;
  }
  float t0 = 1.0 + (d) / dot(cameraObjectFront, vdir);
  if (d == 0) {
    t0 = 0;
  }

  // Calculate clamped limits of integration.
  const float t1 = clamp((pv - m) / v2, t0, 1.0);
  const float t2 = clamp((pv + m) / v2, t0, 1.0);
  const float u1 = t1 * t1;
  const float u2 = t2 * t2;

  // Evaluate density integral, normalize, and square.
  const float B = ((1.0 - p2 * recipR2) * (t2 - t1) + pv * recipR2 * (u2 - u1)
		   - v2 * recip3R2 * (t2 * u2 - t1 * u1)) * normalizer;

  return (B * B * v2);
}

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  const vec2 pixCoord = gl_FragCoord.xy / u_bufferResolution;

  vec3 color = material.diffuse.rgb;
  // color = vec3(0, 0, 1);

  float halo = calculateHaloBrightness(
    fs_in.radius,
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

    float d = 0;
    // float d = textureLod(g_viewZ, pixCoord, 0).x;
    // d = textureLod(g_viewPosition, pixCoord, 0).z;
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

  if (u_forceLineMode) {
    o_fragColor = vec4(1, 0, 0, 1);
  }
}
