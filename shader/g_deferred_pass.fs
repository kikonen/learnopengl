#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl

in VS_OUT {
  vec2 texCoords;
} fs_in;

layout(binding = UNIT_G_ALBEDO_SPEC) uniform sampler2D g_albedoSpec;
layout(binding = UNIT_G_POSITION) uniform sampler2D g_position;
layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
layout(binding = UNIT_G_EMISSION_SHININESS) uniform sampler2D g_emissionShininess;

layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap;


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

void main()
{
  vec3 worldPos = texture(g_position, fs_in.texCoords).rgb;
  vec3 normal = texture(g_normal, fs_in.texCoords).rgb;

  // TODO KI shadowPos needed
  vec4 shadowPos = vec4(worldPos, 1);
  // TODO KI viewPos needed
  vec3 viewPos = worldPos;

  Material material;

  material.diffuse = texture(g_albedoSpec, fs_in.texCoords);
  float specular = material.diffuse.a;
  material.diffuse.a = 1.0;
  material.specular = vec4(specular);

  material.ambient = vec4(0.5, 0.5, 0.5, 1);

  material.emission = texture(g_emissionShininess, fs_in.texCoords);
  material.shininess = material.emission.a;
  material.emission.a = 1.0;

  // TODO KI fogRatio needed (change to be global?!?)
  material.fogRatio = 1.0;

  vec3 toView = normalize(u_viewWorldPos - worldPos);

  vec4 shaded = calculateLight(normal, toView, worldPos, shadowPos, material);
  vec4 texColor = shaded;
  texColor = calculateFog(viewPos, material.fogRatio, texColor);
  texColor = vec4(material.diffuse.rgb + material.emission.rgb, 1.0);

  o_fragColor = texColor;
}
