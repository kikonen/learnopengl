#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec4 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;
  flat float furStrength;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

layout (location = 0) out vec4 o_fragColor;
layout (location = 1) out vec4 o_fragSpecular;
layout (location = 2) out vec4 o_fragEmission;
layout (location = 3) out vec4 o_fragAmbient;
layout (location = 4) out vec3 o_fragPosition;
layout (location = 5) out vec3 o_fragNormal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_normal_pattern.glsl

float getFurAlpha(int noiseTex, vec2 texCoord) {
  sampler2D sampler = sampler2D(u_texture_handles[noiseTex]);
  vec4 noise = texture(sampler, texCoord);
  return clamp(1.0 - noise.r, 0, 1.0);
}

void main() {
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos.xyz);
  #include var_tex_material.glsl

//  material.diffuse.a = getFurAlpha(material.noiseMapTex, fs_in.texCoord);
  material.diffuse.a *= fs_in.furStrength;

#ifdef USE_ALPHA
//  if (material.diffuse.a < 0.1)
//    discard;
#endif

  #include var_tex_material_normal.glsl

  vec4 texColor = material.diffuse;

  o_fragColor = texColor;
  o_fragSpecular = material.specular;
  o_fragSpecular.a = material.shininess;
  o_fragEmission = material.emission;
  o_fragAmbient = material.ambient;

  o_fragPosition = fs_in.worldPos.xyz;
  o_fragNormal = normal;
}
