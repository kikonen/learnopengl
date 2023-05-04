#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl

in VS_OUT {
  vec2 texCoords;
} fs_in;

layout(binding = UNIT_G_ALBEDO) uniform sampler2D g_albedo;
layout(binding = UNIT_G_SPECULAR) uniform sampler2D g_specular;
layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;
layout(binding = UNIT_G_AMBIENT) uniform sampler2D g_ambient;
layout(binding = UNIT_G_POSITION) uniform sampler2D g_position;
layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;

layout(binding = UNIT_OIT_ACCUMULATOR) uniform sampler2D oit_accumulator;
layout(binding = UNIT_OIT_REVEAL) uniform sampler2D oit_reveal;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];


out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_fog.glsl
#include fn_calculate_shadow_index.glsl

const float EPSILON = 0.00001f;

const vec4 CASCADE_COLORS[MAX_SHADOW_MAP_COUNT] =
  vec4[MAX_SHADOW_MAP_COUNT](
          vec4(0.1, 0.0, 0.0, 0.0),
          vec4(0.0, 0.1, 0.0, 0.0),
          vec4(0.0, 0.0, 0.1, 0.0),
          vec4(0.1, 0.0, 0.1, 0.0),
          vec4(0.1, 0.1, 0.0, 0.0)
          );


bool isApproximatelyEqual(float a, float b)
{
  return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(vec3 v)
{
  return max(max(v.x, v.y), v.z);
}

// https://stackoverflow.com/questions/56625730/does-blending-work-with-the-glsl-mix-function
//
// blendMode{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE }
vec4 blend(vec4 source, vec4 dest)
{
  return vec4(source.xyz * source.a + dest.xyz * (1.0 - source.a), 1.0);
}

void main()
{
  const vec3 worldPos = texture(g_position, fs_in.texCoords).rgb;
  const vec3 normal = normalize(texture(g_normal, fs_in.texCoords).rgb);

  const vec3 viewPos = (u_viewMatrix * vec4(worldPos, 1.0)).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);

  const vec3 toView = normalize(u_viewWorldPos - worldPos);

  Material material;

  bool skipLight;
  {
    material.diffuse = texture(g_albedo, fs_in.texCoords);
    // HACK KI alpha == 0.0 is used for skybox
    skipLight = material.diffuse.a == 0.0;
    material.diffuse.a = 1.0;

    material.specular = texture(g_specular, fs_in.texCoords);
    material.shininess = material.specular.a;
    material.specular.a = 1.0;

    material.ambient = texture(g_ambient, fs_in.texCoords);
    material.ambient.a = 1.0;

    material.emission = texture(g_emission, fs_in.texCoords);
    material.emission.a = 1.0;
  }

  if (true) {
    ivec2 fragCoords = ivec2(gl_FragCoord.xy);

    float revealage = texelFetch(oit_reveal, fragCoords, 0).r;
    if (!isApproximatelyEqual(revealage, 1.0f)) {
      vec4 accumulation = texelFetch(oit_accumulator, fragCoords, 0);

      if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

      vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);

      material.diffuse = blend(
        vec4(averageColor, 1.0f - revealage),
        material.diffuse);

      skipLight = false;
    }
  }

  vec4 color;
  if (skipLight) {
    color = material.diffuse;
  } else {
    color = calculateLight(
      normal, toView, worldPos,
      shadowIndex, shadowPos,
      material);

    color = calculateFog(viewPos, color);

    if (u_frustumVisual) {
      color += CASCADE_COLORS[shadowIndex];
    }
  }

  o_fragColor = color;
}
