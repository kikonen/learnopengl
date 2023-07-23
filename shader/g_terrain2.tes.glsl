#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include struct_clip_plane.glsl
#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl
#include uniform_clip_planes.glsl

in TCS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  flat mat3 TBN;
#endif
} tes_in[];

out TES_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  flat mat3 TBN;
#endif

  float height;
} tes_out;


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
  const Entity entity = u_entities[tes_in[0].entityIndex];
  #include var_entity_model_matrix.glsl

  const Material material = u_materials[tes_in[0].materialIndex];
  sampler2D heightMap = sampler2D(u_texture_handles[material.heightMapTex]);

  // Interpolate the attributes of the output vertex using the barycentric coordinates
  vec2 texCoord = interpolate2D(tes_in[0].texCoord, tes_in[1].texCoord, tes_in[2].texCoord);
  vec3 normal = interpolate3D(tes_in[0].normal, tes_in[1].normal, tes_in[2].normal);
  vec3 vertexPos = interpolate3D(tes_in[0].vertexPos, tes_in[1].vertexPos, tes_in[2].vertexPos);


  const float rangeYmin = entity.rangeYmin;
  const float rangeYmax = entity.rangeYmax;
  const float rangeY = rangeYmax - rangeYmin;

  float avgHeight = fetchHeight(heightMap, texCoord);
  float h = rangeYmin + avgHeight * rangeY;

  vertexPos.y += h;

  vec4 worldPos = modelMatrix * vec4(vertexPos, 1.0);

  tes_out.entityIndex = tes_in[0].entityIndex;
  tes_out.worldPos = worldPos.xyz;
  tes_out.normal = normal;
  tes_out.texCoord = texCoord;
  tes_out.vertexPos = vertexPos;
  tes_out.materialIndex = tes_in[0].materialIndex;

#ifdef USE_TBN
  tes_out.TBN = tes_in[0].TBN;
#endif

  tes_out.height = h;

  calculateClipping(worldPos);

  gl_Position = u_projectedMatrix * worldPos;
}
