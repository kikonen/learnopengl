#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

#include screen_tri_vertex_out.glsl

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

uniform vec3 u_samples[64];

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 u_noiseScale = vec2(u_bufferResolution.x / 4.0, u_bufferResolution.y / 4.0);

layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
layout(binding = UNIT_G_VIEW_POSITION) uniform sampler2D g_viewPosition;
// layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;
// layout(binding = UNIT_G_VIEW_Z) uniform sampler2D g_viewZ;

layout(binding = UNIT_NOISE) uniform sampler2D u_noiseTex;

layout (location = 0) out float o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_gbuffer_decode.glsl

// vec3 getViewPos1(const vec2 texCoord)
// {
//   return texture(g_viewZ, texCoord);
// }

// vec3 getViewPos2(const vec2 texCoord)
// {
//   // https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
//   // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
//   vec3 viewPos;
//   {
//     // NOTE KI pixCoord == texCoord in fullscreen quad
//     float depth = texture(g_depth, texCoord).x;

//     const vec4 clip = vec4(
//       texCoord.x * 2.0 - 1.0,
//       texCoord.y * 2.0 - 1.0,
//       depth * 2.0 - 1.0,
//       1.0);
//     vec4 viewW  = u_invProjectionMatrix * clip;
//     viewPos  = viewW.xyz / viewW.w;
//   }

//   return viewPos;
// }

vec3 getViewPos(const vec2 texCoord)
{
  return texture(g_viewPosition, texCoord).xyz;
}

float calculateSsao(
  const vec3 normal,
  const vec2 texCoord)
{
  // get input for SSAO algorithm
  vec3 viewPos = getViewPos(texCoord);
  vec3 randomVec = normalize(texture(u_noiseTex, texCoord * u_noiseScale).xyz);

  // create TBN change-of-basis matrix: from tangent-space to view-space
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);

  // iterate over the sample kernel and calculate occlusion factor
  float occlusion = 0.0;
  for(int i = 0; i < kernelSize; ++i)
  {
    // get sample position
    // from tangent to view-space
    vec3 samplePos = TBN * u_samples[i];
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
    float sampleDepth = getViewPos(offset.xy).z;

    // range check & accumulate
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleDepth));
    occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
  }
  occlusion = 1.0 - (occlusion / kernelSize);

  return occlusion;
}

void main()
{
  #include screen_tri_tex_coord.glsl

  #include var_gbuffer_normal.glsl

  o_fragColor = calculateSsao(normal, texCoord);
}
