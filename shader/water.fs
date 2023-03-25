#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec4 glp;

  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;

#ifdef USE_NORMAL_TEX
  mat3 TBN;
#endif
} fs_in;

layout(binding = UNIT_WATER_NOISE) uniform sampler3D u_noiseTex;
layout(binding = UNIT_WATER_REFLECTION) uniform sampler2D u_reflectionTex;
layout(binding = UNIT_WATER_REFRACTION) uniform sampler2D u_refractionTex;

layout(binding = UNIT_SHADOW_MAP) uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

vec3 estimateWaveNormal(
  in float offset,
  in float mapScale,
  in float hScale)
{
  vec2 tc = fs_in.texCoord;
  // estimate the normal using the noise texture
  // by looking up three height values around this vertex
  float h1 = (texture(u_noiseTex, vec3(((tc.s))*mapScale, 0.5, ((tc.t)+offset)*mapScale))).r * hScale;
  float h2 = (texture(u_noiseTex, vec3(((tc.s)-offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
  float h3 = (texture(u_noiseTex, vec3(((tc.s)+offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
  vec3 v1 = vec3(0, h1, -1);
  vec3 v2 = vec3(-1, h2, 1);
  vec3 v3 = vec3(1, h3, 1);
  vec3 v4 = v2-v1;
  vec3 v5 = v3-v1;
  vec3 normEst = normalize(cross(v4,v5));
  return normEst;
}

const float waveStrength = 0.01;

void main() {
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);

  #include var_tex_material.glsl

  vec2 distortedTexCoord = fs_in.texCoord;
  vec2 totalDistortion = vec2(0);

  if (material.dudvMapTex >= 0) {
    float moveFactor = (sin(u_time / 10.0) + 1.0) * 0.5;

    // distortedTexCoord = texture(u_textures[material.dudvMapTex], vec2(fs_in.texCoord.x + moveFactor, fs_in.texCoord.y)).rg * 0.1;
    // distortedTexCoord = fs_in.texCoord + vec2(distortedTexCoord.x, distortedTexCoord.y + moveFactor);
    // distortedTexCoord = clamp(distortedTexCoord, 0.0, 1.0);
    //totalDistortion = (texture(u_textures[material.dudvMapTex], distortedTexCoord).rg * 2.0 - 1.0) * waveStrength;

    //vec2 distortedTexCoord;
    {
      sampler2D sampler = sampler2D(u_texture_handles[material.dudvMapTex]);
      distortedTexCoord = texture(sampler, vec2(fs_in.texCoord.x + moveFactor, fs_in.texCoord.y)).rg * 0.1;
      distortedTexCoord = fs_in.texCoord + vec2(distortedTexCoord.x, distortedTexCoord.y + moveFactor);
      totalDistortion = (texture(sampler, distortedTexCoord).rg * 2.0 - 1.0) * waveStrength;
    }
  }

#ifdef USE_NORMAL_TEX
  vec3 normal;
  if (material.normalMapTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.normalMapTex]);
    normal = texture(sampler, distortedTexCoord).rgb;

    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
  } else {
    normal = normalize(fs_in.normal);
  }
#else
  vec3 normal = normalize(fs_in.normal);
#endif

  // estimate the normal using the noise texture
  // by looking up three height values around this vertex.
  // input parameters are offset for neighbors, and scaling for width and height
  normal = estimateWaveNormal(.0002, 32.0, 16.0);

  // #include var_calculate_cubemap_diffuse.glsl

  vec4 gp = fs_in.glp;
  vec2 refractCoord = vec2(gp.x, gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;
  vec2 reflectCoord = vec2(gp.x, -gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;

  refractCoord = clamp(refractCoord, 0., 1.0);
  reflectCoord = clamp(reflectCoord, 0., 1.0);

  vec4 refractColor = texture(u_refractionTex, refractCoord);
  vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

  float refractiveFactor = dot(toView, normal);

  if (!gl_FrontFacing) {
    refractiveFactor = 1.0;
  }

  refractiveFactor = clamp(refractiveFactor, 0, 1);

  vec4 mixColor = mix(reflectColor, refractColor, refractiveFactor);

  vec4 origDiffuse = material.diffuse;
  material.diffuse = mix(material.diffuse, mixColor, 0.9);

  vec4 texColor = material.diffuse;
  {
    texColor = calculateLight(normal, toView, fs_in.worldPos, fs_in.shadowPos, material);
    texColor = calculateFog(fs_in.viewPos, material.fogRatio, texColor);
  }

#ifdef USE_BLEND
  o_fragColor = texColor;
#else
  o_fragColor = vec4(texColor.xyz, 1.0);
#endif

  o_fragColor = texColor;
}
