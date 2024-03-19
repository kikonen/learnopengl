#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include ssbo_materials.glsl
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

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} fs_in;

layout(binding = UNIT_WATER_NOISE) uniform sampler3D u_noiseTex;
layout(binding = UNIT_WATER_REFLECTION) uniform sampler2D u_reflectionTex;
layout(binding = UNIT_WATER_REFRACTION) uniform sampler2D u_refractionTex;

layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

vec3 estimateWaveNormal(
  in sampler3D sampler,
  in vec2 tc,
  in float offset,
  in float mapScale,
  in float hScale)
{
  // estimate the normal using the noise texture
  // by looking up three height values around this vertex
  float h1 = (texture(sampler, vec3(((tc.s))*mapScale, 0.5, ((tc.t)+offset)*mapScale))).r * hScale;
  float h2 = (texture(sampler, vec3(((tc.s)-offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
  float h3 = (texture(sampler, vec3(((tc.s)+offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
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
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);

  vec2 distortedTexCoord = texCoord;
  vec2 totalDistortion = vec2(0);

  if (material.dudvMapTex >= 0) {
    float moveFactor = (sin(u_time / 10.0) + 1.0) * 0.5;

    // distortedTexCoord = texture(u_textures[material.dudvMapTex], vec2(texCoord.x + moveFactor, texCoord.y)).rg * 0.1;
    // distortedTexCoord = texCoord + vec2(distortedTexCoord.x, distortedTexCoord.y + moveFactor);
    // distortedTexCoord = clamp(distortedTexCoord, 0.0, 1.0);
    //totalDistortion = (texture(u_textures[material.dudvMapTex], distortedTexCoord).rg * 2.0 - 1.0) * waveStrength;

    //vec2 distortedTexCoord;
    {
      sampler2D sampler = sampler2D(u_texture_handles[material.dudvMapTex]);
      distortedTexCoord = texture(sampler, vec2(texCoord.x + moveFactor, texCoord.y)).rg * 0.1;
      distortedTexCoord = texCoord + vec2(distortedTexCoord.x, distortedTexCoord.y + moveFactor);
      totalDistortion = (texture(sampler, distortedTexCoord).rg * 2.0 - 1.0) * waveStrength;
    }
  }

#ifdef USE_NORMAL_TEX
  vec3 normal;
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    sampler2D sampler = sampler2D(u_materials[materialIndex].normalMapTex);

    const vec3 N = normalize(fs_in.normal);
    const vec3 T = normalize(fs_in.tangent);
    const vec3 B = cross(N, T);
    const mat3 TBN = mat3(T, B, N);

    normal = texture(sampler, distortedTexCoord).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);
  }
#else
  vec3 normal = normalize(fs_in.normal);
#endif

  // estimate the normal using the noise texture
  // by looking up three height values around this vertex.
  // input parameters are offset for neighbors, and scaling for width and height
  normal = estimateWaveNormal(u_noiseTex, texCoord, .0002, 32.0, 16.0);

#ifdef USE_CUBE_MAP
  // #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 gp = fs_in.glp;
  vec2 refractCoord = vec2(gp.x, gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;
  vec2 reflectCoord = vec2(gp.x, -gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;

  refractCoord = clamp(refractCoord, 0., 1.0);
  reflectCoord = clamp(reflectCoord, 0., 1.0);

  vec4 refractColor = texture(u_refractionTex, refractCoord);
  vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

  float refractiveFactor = dot(viewDir, normal);

  if (!gl_FrontFacing) {
    refractiveFactor = 1.0;
  }

  refractiveFactor = clamp(refractiveFactor, 0, 1);

  vec4 mixColor = mix(reflectColor, refractColor, refractiveFactor);

  vec4 origDiffuse = material.diffuse;
  material.diffuse = mix(material.diffuse, mixColor, 0.9);

  vec4 texColor = material.diffuse;
  {
    texColor = calculateLight(
      normal, viewDir, fs_in.worldPos,
      fs_in.shadowIndex,
      material);
    texColor = calculateFog(fs_in.viewPos, texColor);
  }

#ifdef USE_BLEND
  o_fragColor = texColor;
#else
  o_fragColor = vec4(texColor.rgb, 1.0);
#endif

  o_fragColor = texColor;
}
