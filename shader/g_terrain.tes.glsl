#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include struct_clip_plane.glsl
#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

in TCS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;

#ifdef USE_TBN
  vec3 tangent;
#endif
} tes_in[];

out TES_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
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

#include fn_calculate_clipping.glsl


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
  entity = u_entities[tes_in[0].entityIndex];
  #include var_entity_model_matrix.glsl

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

  tes_out.worldPos = worldPos.xyz;
  tes_out.normal = normal;
  tes_out.texCoord = texCoord;
  tes_out.vertexPos = vertexPos;
  tes_out.materialIndex = tes_in[0].materialIndex;

#ifdef USE_TBN
  tes_out.tangent = tes_in[0].tangent;
#endif

  tes_out.height = h;

  calculateClipping(worldPos);

  gl_Position = u_projectedMatrix * worldPos;
}
