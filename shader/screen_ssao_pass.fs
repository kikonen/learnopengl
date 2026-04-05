#version 460 core

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"
#include "include/uniform_buffer_info.glsl"

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

#include "include/screen_tri_vertex_out.glsl"

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
// layout(binding = UNIT_G_VIEW_POSITION) uniform sampler2D g_viewPosition;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;
// layout(binding = UNIT_G_VIEW_Z) uniform sampler2D g_viewZ;

layout(binding = UNIT_NOISE) uniform sampler2D u_noiseTex;

layout (location = 0) out float o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include "include/fn_gbuffer_normal_decode.glsl"
#include "include/fn_gbuffer_depth_decode.glsl"

float calculateSsao(
  float depth,
  const vec3 normal,
  const vec2 texCoord)
{
  // get input for SSAO algorithm
  const vec3 viewPos = getViewPosFromTexCoord(depth, texCoord);

  // Per-pixel procedural noise for kernel rotation
  // Unique rotation per pixel avoids banding from repeating 4x4 texture patterns
  const vec3 randomVec = normalize(vec3(
    fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0,
    fract(sin(dot(gl_FragCoord.xy, vec2(39.3460, 11.135))) * 29816.1923) * 2.0 - 1.0,
    fract(sin(dot(gl_FragCoord.xy, vec2(73.1560, 52.749))) * 17635.8493) * 2.0 - 1.0));

  // create TBN change-of-basis matrix: from tangent-space to view-space
  const vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  const vec3 bitangent = cross(normal, tangent);
  const mat3 TBN = mat3(tangent, bitangent, normal);

  // iterate over the sample kernel and calculate occlusion factor
  float occlusion = 0.0;
  for(int i = 0; i < kernelSize; ++i)
  {
    // get sample position
    // from tangent to view-space
    vec3 samplePos = TBN * u_ssaoSamples[i].xyz;
    samplePos = viewPos + samplePos * radius;

    // project sample position (to sample texture) (to get position on screen/texture)
    vec4 offset = vec4(samplePos, 1.0);

    // from view to clip-space
    offset = u_projectionMatrix * offset;

    // perspective divide
    offset.xyz /= offset.w;

    // transform to range 0.0 - 1.0
    offset.xyz = offset.xyz * 0.5 + 0.5;

    // get sample depth
    // get depth value of kernel sample
    float sampleZ = getViewPosFromGBuffer(offset.xy).z;

    // Smooth depth comparison instead of hard step
    // depthDiff > 0: sample is closer to camera than geometry (no occlusion)
    // depthDiff < 0: geometry is closer than sample (occlusion)
    float depthDiff = samplePos.z - sampleZ;
    float occFactor = 1.0 - smoothstep(-radius * 0.8, bias, depthDiff);

    // range check: reject samples where geometry is too far from original surface
    float rangeCheck = 1.0 - smoothstep(0.0, radius, abs(viewPos.z - sampleZ));
    occlusion += occFactor * rangeCheck;
  }
  occlusion = 1.0 - (occlusion / kernelSize);

  return occlusion;
}

void main()
{
  #include "include/screen_tri_tex_coord.glsl"

  #include "include/var_gbuffer_normal.glsl"

  // NOTE KI pixCoord == texCoord in fullscreen quad
  const float depth = textureLod(g_depth, texCoord, 0).x;
  // NOTE KI skip calculations for skybox
  if (depth >= 1.0) discard;

  o_fragColor = calculateSsao(depth, normal, texCoord);
}
