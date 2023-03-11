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
layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;


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
  vec3 shadowPos = worldPos;

  Material material;
  material.diffuse = texture(g_albedoSpec, fs_in.texCoords).rgb;
  material.diffuse.a = 1.0;
  material.specular = texture(g_albedoSpec, fs_in.texCoords).a;
  material.ambient = vec4(0.5, 0.5, 0.5, 1);

  // TODO KI shininess needed (pass materialIndex?!?)
  material.shininess = 0.0;
  // TODO KI fogRatio needed (change to be global?!?)
  material.fogRatio = 0.0;

  vec3 toView = normalize(u_viewWorldPos - worldPos);

  vec4 shaded = calculateLight(normal, toView, worldPos, shadowPos, material);
  vec4 texColor = shaded;
  texColor = calculateFog(material.fogRatio, texColor);

  o_fragColor = texColor;
}
