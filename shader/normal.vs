#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TANGENT) in vec3 a_tangent;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  vec3 normal;
  flat mat3 TBN;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const mat3 viewNormalMatrix = mat3(transpose(inverse(u_viewMatrix * modelMatrix)));

  vec4 worldPos;

  if ((entity.flags & ENTITY_BILLBOARD_BIT) == ENTITY_BILLBOARD_BIT) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = vec3(modelMatrix[0][0],
                            modelMatrix[1][1],
                            modelMatrix[2][2]);

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + u_viewUp * a_pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = modelMatrix * pos;
  }

  gl_Position = u_viewMatrix * worldPos;

  vs_out.normal = normalize(viewNormalMatrix * a_normal);

  {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize(normalMatrix * a_tangent);
    T = normalize(T - dot(T, N) * N);
    const vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
}
