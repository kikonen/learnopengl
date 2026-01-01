#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include include/ssbo_entities.glsl
#include include/ssbo_instance_indeces.glsl
#include include/ssbo_socket_transforms.glsl

#include include/uniform_matrices.glsl
#include include/uniform_camera.glsl
#include include/uniform_clip_planes.glsl

in TCS_OUT {
  flat uint entityIndex;
  flat uint instanceIndex;

#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} tes_in[];

out TES_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif

  float height;
} tes_out;


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

#include include/fn_calculate_clipping.glsl


vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
  return vec2(gl_TessCoord.x) * v0 +
    vec2(gl_TessCoord.y) * v1 +
    vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
  return vec3(gl_TessCoord.x) * v0 +
    vec3(gl_TessCoord.y) * v1 +
    vec3(gl_TessCoord.z) * v2;
}

float fetchHeight(
  in sampler2D heightMap,
  in vec2 texCoord)
{
  return texture(heightMap, texCoord).r;
}

void main()
{
  instance = u_instances[tes_in[0].instanceIndex];
  entity = u_entities[tes_in[0].entityIndex];
  #include include/var_entity_model_matrix.glsl

  sampler2D heightMap = sampler2D(tes_in[0].heightMapTex);

  // Interpolate the attributes of the output vertex using the barycentric coordinates
  vec2 texCoord = interpolate2D(tes_in[0].texCoord, tes_in[1].texCoord, tes_in[2].texCoord);
  vec3 normal = interpolate3D(tes_in[0].normal, tes_in[1].normal, tes_in[2].normal);
  vec3 vertexPos = interpolate3D(tes_in[0].vertexPos, tes_in[1].vertexPos, tes_in[2].vertexPos);

  const float rangeYmin = tes_in[0].rangeYmin;
  const float rangeYmax = tes_in[0].rangeYmax;
  const float rangeY = rangeYmax - rangeYmin;

  float avgHeight = fetchHeight(heightMap, texCoord);
  float h = rangeYmin + avgHeight * rangeY;

  vertexPos.y += h;

  vec4 worldPos = modelMatrix * vec4(vertexPos, 1.0);

#ifdef USE_CUBE_MAP
  tes_out.worldPos = worldPos.xyz;
#endif
  tes_out.viewPos = (u_viewMatrix * worldPos).xyz;
  tes_out.normal = normal;
  tes_out.texCoord = texCoord;
  tes_out.materialIndex = tes_in[0].materialIndex;

  tes_out.tileIndex = tes_in[0].tileIndex;
  tes_out.tileX = tes_in[0].tileX;
  tes_out.tileY = tes_in[0].tileY;

#ifdef USE_TBN
  tes_out.tbn = tes_in[0].tbn;
#endif
#ifdef USE_PARALLAX
  tes_out.viewTangentPos = tes_in[0].viewTangentPos;
  tes_out.tangentPos = tes_in[0].tangentPos;
#endif

  tes_out.height = h;

  calculateClipping(worldPos);

  gl_Position = u_projectedMatrix * worldPos;
}
