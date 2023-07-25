#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

LAYOUT_G_BUFFER_SAMPLERS;
LAYOUT_OIT_SAMPLERS;

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
#include fn_calculate_fog.glsl
#include fn_calculate_shadow_index.glsl
#include fn_gbuffer_decode.glsl

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

vec4 calculateGlow(vec2 texCoord)
{
  const float offset = 1.0 / 300.0;

  vec2 offsets[9] = vec2[]
      (
       vec2(-offset,  offset), // top-left
       vec2( 0.0f,    offset), // top-center
       vec2( offset,  offset), // top-right
       vec2(-offset,  0.0f),   // center-left
       vec2( 0.0f,    0.0f),   // center-center
       vec2( offset,  0.0f),   // center-right
       vec2(-offset, -offset), // bottom-left
       vec2( 0.0f,   -offset), // bottom-center
       vec2( offset, -offset)  // bottom-right
       );

    // float kernel[9] = float[]
    //   (
    //    1.0 / 16, 2.0 / 16, 1.0 / 16,
    //    2.0 / 16, 4.0 / 16, 2.0 / 16,
    //    1.0 / 16, 2.0 / 16, 1.0 / 16
    //    );

    float kernel[9] = float[]
      (
       0.0, 1.0, 0.0,
       1.0, 3.0, 1.0,
       0.0, 1.0, 0.0
       );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
      sampleTex[i] = vec3(texture(g_emission, texCoord + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
      col += sampleTex[i] * kernel[i];
    }
    return vec4(col, 1.0);
}

void main()
{
  const vec2 texCoord = fs_in.texCoord;

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

  //const vec3 worldPos = texture(g_position, texCoord).rgb;
  #include var_gbuffer_normal.glsl

  //const vec3 viewPos = (u_viewMatrix * vec4(worldPos, 1.0)).xyz;

  const uint shadowIndex = calculateShadowIndex(viewPos);

  const vec3 viewDir = normalize(u_viewWorldPos - worldPos);

  Material material;

  bool skipLight;
  {
    material.diffuse = texture(g_albedo, texCoord);
    material.ambient = material.diffuse.a;
    material.diffuse.a = 1.0;

    material.specular = texture(g_specular, texCoord);
    material.emission = texture(g_emission, texCoord).xyz;
  }

  if (true) {
    float revealage = texture(oit_reveal, texCoord).r;
    if (!isApproximatelyEqual(revealage, 1.0f)) {
      vec4 accumulation = texture(oit_accumulator, texCoord, 0);

      if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

      vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);

      material.diffuse = blend(
        vec4(averageColor, 1.0f - revealage),
        material.diffuse);

      skipLight = false;
    }
  }

  if (material.ambient >= 1.0) {
    skipLight = true;
  }

  vec4 color;
  if (skipLight) {
    color = material.diffuse;
  } else {
    color = calculateLight(
      normal, viewDir, worldPos,
      shadowIndex,
      material);

    color = calculateFog(viewPos, color);

    if (u_frustumVisual) {
      color += CASCADE_COLORS[shadowIndex];
    }
  }

  o_fragColor = color;
}
