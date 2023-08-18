#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec4 glp;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} fs_in;

//layout(binding = UNIT_WATER_NOISE) uniform sampler3D u_noiseTex;
layout(binding = UNIT_WATER_REFLECTION) uniform sampler2D u_reflectionTex;
layout(binding = UNIT_WATER_REFRACTION) uniform sampler2D u_refractionTex;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl
#include fn_gbuffer_encode.glsl

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
  Material material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_plain_material.glsl

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
  if (material.normalMapTex >= 0) {
    sampler2D sampler = sampler2D(u_texture_handles[material.normalMapTex]);

    const vec3 N = normalize(fs_in.normal);
    const vec3 T = normalize(fs_in.tangent);
    const vec3 B = cross(N, T);
    const mat3 TBN = mat3(T, B, N);

    normal = texture(sampler, distortedTexCoord).rgb;
    normal = normalize(TBN * normal);
  } else {
    // NOTE KI model *can* have multiple materials; some with normalTex
    normal = normalize(fs_in.normal);
  }
#else
  vec3 normal = normalize(fs_in.normal);
#endif

  // estimate the normal using the noise texture
  // by looking up three height values around this vertex.
  // input parameters are offset for neighbors, and scaling for width and height
  //normal = estimateWaveNormal(u_noiseTex, texCoord, .0002, 32.0, 16.0);

  vec2 ndc = (fs_in.glp.xy / fs_in.glp.w) / 2.0 + 0.5;

  vec2 refractCoord = vec2(ndc.x,  ndc.y) + totalDistortion;
  vec2 reflectCoord = vec2(ndc.x, -ndc.y) + totalDistortion;

  // NOTE KI do NOT do clamping of coords; breaks logic
  //refractCoord = clamp(refractCoord, 0., 1.0);
  //reflectCoord = clamp(reflectCoord, 0., 1.0);

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

  o_fragColor = vec4(texColor.xyz, material.ambient);
  o_fragSpecular = material.specular;
  o_fragEmission = material.emission.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}
