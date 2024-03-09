#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl
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

layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
layout(binding = UNIT_PREFILTER_MAP) uniform samplerCube u_prefilterMap;
layout(binding = UNIT_BRDF_LUT) uniform sampler2D u_brdfLut;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];


layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragBright;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

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
          vec4(0.2, 0.0, 0.2, 0.0),
          vec4(0.2, 0.2, 0.0, 0.0)
          );

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  // https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
  // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
  vec3 worldPos;
  vec3 viewPos;
  {
    float depth = textureLod(g_depth, texCoord, 0).x * 2.0 - 1.0;

    vec4 clip = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, depth, 1.0);
    vec4 viewW  = u_invProjectionMatrix * clip;
    viewPos  = viewW.xyz / viewW.w;
    worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  }

  #include var_gbuffer_normal.glsl

  const uint shadowIndex = calculateShadowIndex(viewPos);

  const vec3 viewDir = normalize(u_mainViewWorldPos - worldPos);

  {
    material.diffuse = textureLod(g_albedo, texCoord, 0);
    material.diffuse.a = 1.0;

    material.metal = textureLod(g_metal, texCoord, 0);
    material.emission = textureLod(g_emission, texCoord, 0);
    material.emission.a = 1.0;

    material.metal = textureLod(g_metal, texCoord, 0);
  }

  vec4 color;
  bool emission = false;
  {
    color = calculateLightPbr(
      normal, viewDir, worldPos,
      shadowIndex);

    emission = (material.emission.r + material.emission.g + material.emission.b) > 0;

    if (u_shadowVisual) {
      color += CASCADE_COLORS[shadowIndex];
    }
  }

  const vec3 T = vec3(0.2126, 0.7152, 0.0722);
  const float brightness = 0;//dot(color.xyz, T);

  if (emission) {
    o_fragBright = vec4(material.emission.xyz, 1.0);
  } else if (brightness > 1.0) {
    o_fragBright = vec4(color.xyz, 1.0);
  } else {
    o_fragBright = vec4(0.0, 0.0, 0.0, 1.0);
  }

  o_fragColor = color;
}
