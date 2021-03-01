#version 450 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

in VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat int materialIndex;

  vec4 fragPosLightSpace;

  mat3 TBN;
} fs_in;

uniform sampler2D textures[TEX_COUNT];
// uniform samplerCube reflectionMap;
// uniform samplerCube refractionMap;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler3D noiseTex;

uniform sampler2DShadow shadowMap;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

vec3 estimateWaveNormal(float offset, float mapScale, float hScale) {
  vec2 tc = fs_in.texCoords;
  // estimate the normal using the noise texture
  // by looking up three height values around this vertex
  float h1 = (texture(noiseTex, vec3(((tc.s))*mapScale, 0.5, ((tc.t)+offset)*mapScale))).r * hScale;
  float h2 = (texture(noiseTex, vec3(((tc.s)-offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
  float h3 = (texture(noiseTex, vec3(((tc.s)+offset)*mapScale, 0.5, ((tc.t)-offset)*mapScale))).r * hScale;
  vec3 v1 = vec3(0, h1, -1);
  vec3 v2 = vec3(-1, h2, 1);
  vec3 v3 = vec3(1, h3, 1);
  vec3 v4 = v2-v1;
  vec3 v5 = v3-v1;
  vec3 normEst = normalize(cross(v4,v5));
  return normEst;
}

void main() {
  #include var_tex_material.glsl

  vec3 normal;
  if (material.normalMapTex >= 0) {
    normal = texture(textures[material.normalMapTex], fs_in.texCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
  } else {
    normal = fs_in.normal;
  }

  if (material.pattern == 1) {
    normal = calculateNormalPattern(normal);
  }

  // estimate the normal using the noise texture
  // by looking up three height values around this vertex.
  // input parameters are offset for neighbors, and scaling for width and height
//  normal = estimateWaveNormal(.0002, 32.0, 16.0);

  if (!gl_FrontFacing) {
    normal = -normal;
  }

  vec3 toView = normalize(viewPos - fs_in.fragPos);

//  #include var_calculate_diffuse.glsl

  // vec2 ndc = (fs_in.glp.xy / (fs_in.glp.w * 2.0)) + 0.5;
  // vec2 refractTexCoords = ndc;
  // vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);
  // vec4 refractColor = texture(refractionTex, refractTexCoords);
  // vec4 reflectColor = texture(reflectionTex, reflectTexCoords);

  vec4 refractColor = texture(refractionTex, (vec2(fs_in.glp.x, fs_in.glp.y)) / (2.0 * fs_in.glp.w) + 0.5);
  vec4 reflectColor = texture(reflectionTex, (vec2(fs_in.glp.x, -fs_in.glp.y)) / (2.0 * fs_in.glp.w) + 0.5);

  vec4 mixColor = mix(reflectColor, refractColor, 0.2);

  material.diffuse = mix(material.diffuse, mixColor, 0.9);

  vec4 shaded = calculateLight(normal, toView, material);
  vec4 texColor = shaded;

  texColor = calculateFog(material.fogRatio, texColor);

//  texColor = vec4(1, 0, 0, 1);

  fragColor = texColor;
}
