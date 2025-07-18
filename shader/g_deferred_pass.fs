#version 460 core

#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl
#include uniform_lights.glsl

// NOTE KI depth is *not* used
// => for *stencil test
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

#include screen_tri_vertex_out.glsl

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

void main()
{
  #include screen_tri_tex_coord.glsl

  vec3 worldPos;
  vec3 viewPos;
  if (true) {
    viewPos = getViewPosFromGBuffer(texCoord);
    worldPos = getWorldPosFromViewPos(viewPos);
  }
  // if (false) {
  //   viewPos = texture(g_viewPosition, texCoord).xyz;
  //   worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  // }

  #include var_gbuffer_normal.glsl

  const uint shadowIndex = calculateShadowIndex(viewPos);

  const vec3 viewDir = normalize(u_mainCameraPos.xyz - worldPos);

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

  // color.rgb = vec3(material.ssao);

  if (false)
  {
    float z = 0;
    float dp = 0;

    // dp = textureLod(g_depth, texCoord, 0).x;
    // float depth = linearizeDepth(dp);
    // dp = 1.0 - (dp - 0.99) * 100.0;

    // z = textureLod(g_viewPosition, texCoord, 0).z;
    // dp = -textureLod(g_viewPosition, texCoord, 0).z;

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
