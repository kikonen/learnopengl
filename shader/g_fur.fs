#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec3 worldPos;
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

Material material;

#include fn_gbuffer_encode.glsl

void main() {
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  sampler2D sampler = sampler2D(material.noiseMapTex);
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

  o_fragColor = vec4(texColor.xyz, 1.0);
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}
