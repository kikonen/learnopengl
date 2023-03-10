#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec2 texCoords;
} fs_in;

layout(binding = UNIT_G_ALBEDO_SPEC) uniform sampler2D g_albedoSpec;
layout(binding = UNIT_G_POSITION) uniform sampler2D g_position;
layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;


out vec4 o_fragColor;

void main()
{
  // retrieve data from gbuffer
  vec3 worldPos = texture(g_position, fs_in.texCoords).rgb;
  vec3 normal = texture(g_normal, fs_in.texCoords).rgb;
  vec3 diffuse = texture(g_albedoSpec, fs_in.texCoords).rgb;
  float specular = texture(g_albedoSpec, fs_in.texCoords).a;

//  diffuse = vec3(1, 1, 0);

  o_fragColor = vec4(diffuse, 1.0);
}
