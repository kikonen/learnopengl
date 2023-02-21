#version 460 core

layout(quads, fractional_odd_spacing, ccw) in;

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
  vec4 vertexPos;
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
  vec4 vertexPos;
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

void main()
{
  const Entity entity = u_entities[tes_in[0].entityIndex];
  #include var_entity_model_matrix.glsl

  const Material material = u_materials[tes_in[0].materialIndex];
  sampler2D heightMap = sampler2D(u_texture_handles[material.heightMapTex]);

  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  vec2 t00 = tes_in[0].texCoord;
  vec2 t01 = tes_in[1].texCoord;
  vec2 t10 = tes_in[2].texCoord;
  vec2 t11 = tes_in[3].texCoord;

  vec2 t0 = (t01 - t00) * u + t00;
  vec2 t1 = (t11 - t10) * u + t10;
  vec2 texCoord = (t1 - t0) * v + t0;

  tes_out.entityIndex = tes_in[0].entityIndex;
  tes_out.worldPos = tes_in[0].worldPos;
  tes_out.normal = tes_in[0].normal;
  tes_out.texCoord = texCoord; //tes_in[0].texCoord;
  tes_out.vertexPos = tes_in[0].vertexPos;
  tes_out.viewPos = tes_in[0].viewPos;
  tes_out.materialIndex = tes_in[0].materialIndex;
  tes_out.shadowPos = tes_in[0].shadowPos;

#ifdef USE_NORMAL_TEX
  tes_out.TBN = tes_in[0].TBN;
#endif

  tes_out.height = texture(heightMap, texCoord).r * 64.0 - 16.0;

  vec4 p00 = gl_in[0].gl_Position;
  vec4 p01 = gl_in[1].gl_Position;
  vec4 p10 = gl_in[2].gl_Position;
  vec4 p11 = gl_in[3].gl_Position;

  vec4 uVec = p01 - p00;
  vec4 vVec = p10 - p00;
  vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

  vec4 p0 = (p01 - p00) * u + p00;
  vec4 p1 = (p11 - p10) * u + p10;
  vec4 p = (p1 - p0) * v + p0 + normal * tes_out.height;

  gl_Position = u_projectedMatrix * modelMatrix * p;
}
