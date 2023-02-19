#version 460 core

layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;

  flat uint materialIndex;

  vec3 scale;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_out;

//out float gl_ClipDistance[CLIP_COUNT];

const vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);
const vec3 normal = vec3(0.0, 1.0, 0.0);
const vec3 tangent = vec3(0.0, 0.0, 1.0);


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

//#include fn_calculate_clipping.glsl

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.materialIndex;
  const vec4 worldPos = modelMatrix * pos;

  gl_Position = worldPos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * normal;

  vs_out.scale = vec3(modelMatrix[0][0],
                      modelMatrix[1][1],
                      modelMatrix[2][2]);

  //calculateClipping(worldPos);

  //vs_out.shadowPos = u_shadowMatrix * worldPos;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0)
  {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize(normalMatrix * tangent);
    T = normalize(T - dot(T, N) * N);
    const vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
