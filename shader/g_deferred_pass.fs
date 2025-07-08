#version 460 core

#include struct_lights.glsl

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl
#include uniform_lights.glsl

// NOTE KI depth is *not* used
// => for *stencil test
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;

LAYOUT_G_BUFFER_SAMPLERS;

layout(binding = UNIT_SSAO_BLUR) uniform sampler2D u_ssaoBlurTex;

layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
layout(binding = UNIT_PREFILTER_MAP) uniform samplerCube u_prefilterMap;
layout(binding = UNIT_BRDF_LUT) uniform sampler2D u_brdfLut;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];


layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include pbr.glsl
#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_shadow_index.glsl
#include fn_gbuffer_decode.glsl

const vec4 CASCADE_COLORS[MAX_SHADOW_MAP_COUNT_ABS] =
  vec4[MAX_SHADOW_MAP_COUNT_ABS](
          vec4(0.2, 0.0, 0.0, 0.0),
          vec4(0.0, 0.2, 0.0, 0.0),
          vec4(0.0, 0.0, 0.5, 0.0),
          vec4(0.2, 0.0, 0.2, 0.0)
          );

float linearizeDepth(float depth) {
  return linearizeDepth(depth, u_nearPlane, u_farPlane);
}

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  vec2 texCoord = fs_in.texCoord;

  // const vec2 pixCoord = gl_FragCoord.xy / u_bufferResolution;
  // texCoord = pixCoord;

  // https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
  // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
  vec3 worldPos;
  vec3 viewPos;
  float depth;
  {
    // NOTE KI pixCoord == texCoord in fullscreen quad
    const vec2 pixCoord = texCoord;
    depth = textureLod(g_depth, pixCoord, 0).x;

    const vec4 clip = vec4(
      pixCoord.x * 2.0 - 1.0,
      pixCoord.y * 2.0 - 1.0,
      depth * 2.0 - 1.0,
      1.0);
    vec4 viewW  = u_invProjectionMatrix * clip;
    viewPos  = viewW.xyz / viewW.w;
    worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  }

  #include var_gbuffer_normal.glsl

  const uint shadowIndex = calculateShadowIndex(viewPos);

  const vec3 viewDir = normalize(u_mainCameraPos - worldPos);

  {
    material.diffuse = textureLod(g_albedo, texCoord, 0);
    material.emission = textureLod(g_emission, texCoord, 0).rgb;
    material.diffuse.a = 1.0;
    material.mra = textureLod(g_mra, texCoord, 0).rgb;

    material.ssao = texture(u_ssaoBlurTex, texCoord).r;
  }

  vec4 color;
  {
    if (Debug.u_lightEnabled) {
      color = calculateLightPbr(
	normal, viewDir, worldPos,
	shadowIndex);
    } else {
      color = material.diffuse;
      color.rgb += material.emission;
    }

    if (u_forceLineMode) {
      color = material.diffuse;
    }

    if (u_shadowVisual) {
      color += CASCADE_COLORS[shadowIndex];
    }
  }

  if (false)
  {
    // float dp = textureLod(g_depth, texCoord, 0).x;
    // float depth = linearizeDepth(dp);
    // dp = 1.0 - (dp - 0.99) * 100.0;

    float z = textureLod(g_viewZ, texCoord, 0).x;
    float dp = -textureLod(g_viewZ, texCoord, 0).x;
    dp /= u_farPlane;
    dp = 1.0 - dp;

    color.rgb = vec3(dp);
    if (z < 0) {
      color.rgb = vec3(1, 0, 0);
    }
  }

  o_fragColor = color;

  // o_fragColor = vec4(normal, 1.0);
  // o_fragColor = vec4(abs(normal), 1.0);
  // o_fragColor.a = 1.0;
  // o_fragColor.rgb = vec3(-normal.x);
}
