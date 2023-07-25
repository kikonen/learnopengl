#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl
#include uniform_lights.glsl

LAYOUT_G_BUFFER_SAMPLERS;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];


out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_shadow_index.glsl
#include fn_gbuffer_decode.glsl

const vec4 CASCADE_COLORS[MAX_SHADOW_MAP_COUNT] =
  vec4[MAX_SHADOW_MAP_COUNT](
          vec4(0.1, 0.0, 0.0, 0.0),
          vec4(0.0, 0.1, 0.0, 0.0),
          vec4(0.0, 0.0, 0.1, 0.0),
          vec4(0.1, 0.0, 0.1, 0.0),
          vec4(0.1, 0.1, 0.0, 0.0)
          );

void main()
{
  const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;

  // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
  vec3 worldPos;
  vec3 viewPos;
  {
    float depth = texture(g_depth, texCoord).x * 2.0 - 1.0;

    vec4 clip = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, depth, 1.0);
    vec4 viewW  = u_invProjectionMatrix * clip;
    viewPos  = viewW.xyz / viewW.w;
    worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  }

  #include var_gbuffer_normal.glsl

  const uint shadowIndex = calculateShadowIndex(viewPos);

  const vec3 viewDir = normalize(u_viewWorldPos - worldPos);

  Material material;

  {
    material.diffuse = texture(g_albedo, texCoord);
    material.ambient = material.diffuse.a;
    material.diffuse.a = 1.0;

    material.specular = texture(g_specular, texCoord);
    material.emission = vec3(0);
  }

  bool skipLight = material.ambient >= 1.0;

  vec4 color;
  if (skipLight) {
    color = material.diffuse;
  } else {
    color = calculateLight(
      normal, viewDir, worldPos,
      shadowIndex,
      material);

    if (u_frustumVisual) {
      color += CASCADE_COLORS[shadowIndex];
    }
  }

  o_fragColor = color;
}
