#version 450 core

#include constants.glsl

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

#ifdef USE_NORMAL_TEX
  mat3 TBN;
#endif
} fs_in;

//uniform sampler2D u_textures[TEX_COUNT];

uniform sampler2D u_reflectionTex;
uniform sampler2D u_refractionTex;
uniform sampler3D u_noiseTex;

uniform sampler2DShadow u_shadowMap;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

vec3 estimateWaveNormal(float offset, float mapScale, float hScale) {
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
  #include var_tex_material.glsl

  vec2 distortedTexCoord = fs_in.texCoord;
  vec2 totalDistortion = vec2(0);

  if (material.dudvMapTex >= 0) {
    float moveFactor = (sin(u_time / 10.0) + 1.0) * 0.5;

    vec2 distortedTexCoord = texture(u_textures[material.dudvMapTex], vec2(fs_in.texCoord.x + moveFactor, fs_in.texCoord.y)).rg * 0.1;
    distortedTexCoord = fs_in.texCoord + vec2(distortedTexCoord.x, distortedTexCoord.y + moveFactor);
    totalDistortion = (texture(u_textures[material.dudvMapTex], distortedTexCoord).rg * 2.0 - 1.0) * waveStrength;
  }

#ifdef USE_NORMAL_TEX
  vec3 normal = texture(u_textures[material.normalMapTex], distortedTexCoord).rgb;
  normal = normal * 2.0 - 1.0;
  normal = normalize(fs_in.TBN * normal);
#else
  vec3 normal = fs_in.normal;
#endif

  // estimate the normal using the noise texture
  // by looking up three height values around this vertex.
  // input parameters are offset for neighbors, and scaling for width and height
//  normal = estimateWaveNormal(.0002, 32.0, 16.0);

  if (!gl_FrontFacing) {
    normal = -normal;
  }

  vec3 toView = normalize(u_viewPos - fs_in.fragPos);

//  #include var_calculate_diffuse.glsl

  vec4 gp = fs_in.glp;
  vec2 refractCoord = vec2(gp.x, gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;
  vec2 reflectCoord = vec2(gp.x, -gp.y) / (gp.w * 2.0) + 0.5 + totalDistortion;

  refractCoord = clamp(refractCoord, 0., 1.0);
  reflectCoord = clamp(reflectCoord, 0., 1.0);

//  reflectCoord.x = clamp(reflectCoord.x, 0.001, 0.999);
//  reflectCoord.y = clamp(reflectCoord.y, -0.999, -0.001);

  vec4 refractColor = texture(u_refractionTex, refractCoord);
  vec4 reflectColor = texture(u_reflectionTex, reflectCoord);

  float refractiveFactor = dot(toView, normal);
  refractiveFactor = pow(refractiveFactor, 4);

  vec4 mixColor = mix(reflectColor, refractColor, refractiveFactor);

  vec4 origDiffuse = material.diffuse;
  material.diffuse = mix(material.diffuse, mixColor, 0.9);

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  texColor = calculateFog(material.fogRatio, texColor);

#ifdef USE_BLEND
  fragColor = texColor;
#else
  fragColor = vec4(texColor.xyz, 1.0);
#endif
}
