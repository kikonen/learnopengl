#version 460 core

layout(triangles, fractional_odd_spacing, ccw) in;

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

in TCS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} tes_in[];

out TES_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif

  float height;
} tes_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

#include fn_calculate_shadow_index.glsl


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

  float h = rangeYmin + texture(heightMap, texCoord).r * rangeY;

  vertexPos.y += h;

  vec4 worldPos = modelMatrix * vec4(vertexPos, 1.0);

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

  tes_out.entityIndex = tes_in[0].entityIndex;
  tes_out.worldPos = worldPos.xyz;
  tes_out.normal = normal;
  tes_out.texCoord = texCoord;
  tes_out.vertexPos = vertexPos;
  tes_out.viewPos = (u_viewMatrix * worldPos).xyz;
  tes_out.materialIndex = tes_in[0].materialIndex;
  tes_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

#ifdef USE_NORMAL_TEX
  tes_out.TBN = tes_in[0].TBN;
#endif

  tes_out.height = h;

  gl_Position = u_projectedMatrix * worldPos;
}
