#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl

in VS_OUT {
  vec3 worldPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  flat uint materialIndex;
  flat float furStrength;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  sampler2D sampler = sampler2D(u_materials[materialIndex].noiseMapTex);
  vec4 noiseColor = texture(sampler, texCoord * 8.0);
  float noise = noiseColor.r;

  float alpha = material.diffuse.a;
  float t = material.diffuse.a;
//  t *= fs_in.furStrength * noise;
  t = noise;

  if (t < 0.09)
    discard;

  t *= 1.5;
  t = clamp(t, 0, 1);

  const vec3 normal = normalize(fs_in.normal);

  // NOTE KI fake shadow
  float shadow = mix(0.4, 1.0, 1.0 - fs_in.furStrength);
  vec4 texColor = material.diffuse * shadow;
  texColor.a = alpha;

  texColor *= vec4(1.0, 1.0, 1.0, t);

  o_fragColor = texColor.rgb;
  o_fragMRA = material.mra;
  o_fragEmission = material.emission;

  #include encode_gbuffer_normal.glsl
  #include encode_gbuffer_view_position.glsl
  #include encode_gbuffer_view_z.glsl
}
