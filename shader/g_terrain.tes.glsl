#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_clip_planes.glsl"

in TCS_OUT {
  flat uint entityIndex;
  flat uint instanceIndex;

  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  vec3 objectPos;

  flat uint materialIndex;

  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;

} tes_in[];

out TES_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  vec3 objectPos;

  flat uint materialIndex;
  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

#ifdef USE_TBN
  // xyz = tangent (view space, built from heightmap gradient); w = handedness.
  vec4 tangent;
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

#include "include/fn_calculate_clipping.glsl"


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

#include "include/fn_terrain_height.glsl"

void main()
{
  instance = u_instances[tes_in[0].instanceIndex];
  entity = u_entities[tes_in[0].entityIndex];
  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  sampler2D heightMap = sampler2D(tes_in[0].heightMapTex);

  // Interpolate the attributes of the output vertex using the barycentric coordinates
  vec2 texCoord = interpolate2D(tes_in[0].texCoord, tes_in[1].texCoord, tes_in[2].texCoord);
  vec3 vertexPos = interpolate3D(tes_in[0].objectPos, tes_in[1].objectPos, tes_in[2].objectPos);

  const float rangeYmin = tes_in[0].rangeYmin;
  const float rangeYmax = tes_in[0].rangeYmax;
  const float rangeY = rangeYmax - rangeYmin;

  float avgHeight = fetchHeight(heightMap, texCoord);
  float h = rangeYmin + avgHeight * rangeY;

  vertexPos.y += h;

  // Compute normal from heightmap gradient
  // Mesh is XZ plane [-1,1] with texCoord [0,1], so 1 texCoord unit = 2 object units
  vec3 normal;
  vec3 objTangent;
  {
    ivec2 texSize = textureSize(heightMap, 0);
    vec2 texelSize = 1.0 / vec2(texSize);

    float hL = fetchHeight(heightMap, texCoord + vec2(-texelSize.x, 0));
    float hR = fetchHeight(heightMap, texCoord + vec2( texelSize.x, 0));
    float hD = fetchHeight(heightMap, texCoord + vec2(0, -texelSize.y));
    float hU = fetchHeight(heightMap, texCoord + vec2(0,  texelSize.y));

    // Object-space tangent vectors for the displaced surface:
    // T_u = (2, dh/du * rangeY, 0), T_v = (0, dh/dv * rangeY, 2)
    // normal = normalize(cross(T_v, T_u)) to point up
    float dhdx = (hR - hL) * rangeY * float(texSize.x) * 0.5;
    float dhdz = (hU - hD) * rangeY * float(texSize.y) * 0.5;
    normal = normalize(vec3(-dhdx, 2.0, -dhdz));
    objTangent = normalize(vec3(1.0, dhdx / float(texSize.x), 0.0));
  }

  vec4 worldPos = modelMatrix * vec4(vertexPos, 1.0);

  vec3 viewNormal = normalize(viewNormalMatrix * normal);

  tes_out.viewPos = (u_viewMatrix * worldPos).xyz;
  tes_out.normal = viewNormal;
  tes_out.texCoord = texCoord;
  tes_out.objectPos = vertexPos;
  tes_out.materialIndex = tes_in[0].materialIndex;

  tes_out.tileIndex = tes_in[0].tileIndex;
  tes_out.tileX = tes_in[0].tileX;
  tes_out.tileY = tes_in[0].tileY;

#ifdef USE_TBN
  {
    vec3 viewTangent = normalize(viewNormalMatrix * objTangent);
    // Heightmap tangent is right-handed against the generated normal.
    tes_out.tangent = vec4(viewTangent, 1.0);
  }
#endif

  tes_out.height = h;

  calculateClipping(worldPos);

  gl_Position = u_projectedMatrix * worldPos;
}
