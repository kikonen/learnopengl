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
  vec3 tangent;
  vec3 bitangent;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const mat3 viewNormalMatrix = mat3(transpose(inverse(u_viewMatrix * modelMatrix)));

  vec4 worldPos;
  vec3 normal;
  vec3 tangent;

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((entity.u_flags & ENTITY_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);

    normal = -u_viewFront;
    tangent = u_viewRight;
  } else if ((entity.u_flags & ENTITY_SPRITE_BIT) != 0) {
    vec4 pos = vec4(u_viewRight * a_pos.x
		    + UP * a_pos.y,
		    1.0);

    worldPos = modelMatrix * pos;

    normal = -u_viewFront;
    tangent = u_viewRight;
  } else {
    worldPos = modelMatrix * pos;

    normal = normalize(viewNormalMatrix * a_normal);
    tangent = normalize(viewNormalMatrix * a_tangent);
  }

  gl_Position = u_viewMatrix * worldPos;

  vs_out.normal = normal;

  {
    const vec3 N = vs_out.normal;
    vec3 T = tangent;

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    const vec3 B = cross(N, T);

    vs_out.tangent = T;
    vs_out.bitangent = B;
  }
}
