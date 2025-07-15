#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_clip_planes.glsl

in TCS_OUT {
  flat mat4 modelMatrix;

  vec3 worldPos;
  vec2 texCoord;
  vec3 vertexPos;

  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;
} tes_in[];


out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

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
  const vec2 ts = 1.0 / vec2(textureSize(heightMap, 0));

  float height = 0.0;
  for (int x = -1; x < 2; x++) {
    for (int y = -1; y < 2; y++) {
      height += texture(heightMap, texCoord + vec2(x * ts.x, y * ts.y)).r;
    }
  }
  return height / 9.0;
}

void main()
{
  sampler2D heightMap = sampler2D(tes_in[0].heightMapTex);

  // Interpolate the attributes of the output vertex using the barycentric coordinates
  vec2 texCoord = interpolate2D(tes_in[0].texCoord, tes_in[1].texCoord, tes_in[2].texCoord);
  vec3 vertexPos = interpolate3D(tes_in[0].vertexPos, tes_in[1].vertexPos, tes_in[2].vertexPos);


  const float rangeYmin = tes_in[0].rangeYmin;
  const float rangeYmax = tes_in[0].rangeYmax;
  const float rangeY = rangeYmax - rangeYmin;

  float avgHeight = fetchHeight(heightMap, texCoord);
  float h = rangeYmin + avgHeight * rangeY;

  vertexPos.y += h;

  vec4 worldPos = tes_in[0].modelMatrix * vec4(vertexPos, 1.0);

  calculateClipping(worldPos);

  gl_Position = u_projectedMatrix * worldPos;
}
