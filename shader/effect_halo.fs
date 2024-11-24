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
  const in vec2 pixelCoord)
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
  // const vec2 depth = vec2(1, 1); //texture(structureBuffer, pixelCoord).zw;
  const float depth = 1000;//textureLod(g_depth, pixelCoord, 0).x * 2.0 - 1.0;
  float t0 = 1.0 + (depth) / dot(cameraObjectFront, vdir);
  t0 = 0;

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

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec2 pixelCoord = vec2(
    gl_FragCoord.x / u_bufferResolution.x,
    gl_FragCoord.y / u_bufferResolution.y);

  vec4 color = material.diffuse;

  float halo = calculateHaloBrightness(
    fs_in.radius,
    fs_in.cameraObjectPos,
    fs_in.cameraObjectFront,
    fs_in.pos,
    pixelCoord);
  // halo *= 5;
  // halo = 1;
  float blend = halo;
  // if (halo < 0.1) {
  //   color = vec4(0, 0, 0, 1);
  //   color.rg = fs_in.texCoord;
  //   blend = 1;
  // }

#ifdef USE_BLEND
  o_fragColor = vec4(color.rgb, blend);
#else
  o_fragColor = vec4(color.rgb, 1);
#endif
}
