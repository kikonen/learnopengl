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

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2D u_shadowMap[MAX_SHADOW_MAP_COUNT];


out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_fog.glsl
#include fn_calculate_shadow_index.glsl

const vec4 CASCADE_COLORS[3] =
  vec4[3](
          vec4(0.1, 0.0, 0.0, 0.0),
          vec4(0.0, 0.1, 0.0, 0.0),
          vec4(0.0, 0.0, 0.1, 0.0)
          );

void main()
{
  const vec3 worldPos = texture(g_position, fs_in.texCoords).rgb;
  const vec3 normal = normalize(texture(g_normal, fs_in.texCoords).rgb);

  const vec3 viewPos = (u_viewMatrix * vec4(worldPos, 1.0)).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);

  const vec3 toView = normalize(u_viewWorldPos - worldPos);

  Material material;
  {
    material.diffuse = texture(g_albedo, fs_in.texCoords);
    material.diffuse.a = 1.0;

    material.specular = texture(g_specular, fs_in.texCoords);
    material.shininess = material.specular.a;
    material.specular.a = 1.0;

    material.ambient = texture(g_ambient, fs_in.texCoords);
    material.ambient.a = 1.0;

    material.emission = texture(g_emission, fs_in.texCoords);
    material.emission.a = 1.0;

    // NOTE KI fogRatio is global only now
    material.fogRatio = u_fogRatio;
  }

  vec4 color = calculateLight(normal, toView, worldPos, shadowIndex, shadowPos, material);
  color = calculateFog(viewPos, material.fogRatio, color);

//  color += CASCADE_COLORS[shadowIndex];

  o_fragColor = color;
}
