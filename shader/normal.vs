#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TANGENT) in vec3 a_tangent;

#include tech_skinned_mesh_data.glsl

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_socket_transforms.glsl

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

Instance instance;
Entity entity;

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;
  const mat3 viewNormalMatrix = mat3(transpose(inverse(u_viewMatrix * modelMatrix)));

  vec4 pos = vec4(a_pos, 1.0);

  vec4 worldPos;
  vec3 normal;
  vec3 tangent;

  // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
  // - "ogl" approach
  if ((instance.u_flags & INSTANCE_BILLBOARD_BIT) != 0) {
    vec3 entityPos = vec3(modelMatrix[3]);
    vec3 entityScale = entity.u_worldScale.xyz;

    worldPos = vec4(entityPos
                    + u_cameraRight * a_pos.x * entityScale.x
                    + UP * a_pos.y * entityScale.y,
                    1.0);

    normal = -u_cameraFront;
    tangent = u_cameraRight;
  } else {
    normal = a_normal;
    tangent = a_tangent;

    #include tech_skinned_mesh_skin.glsl

    worldPos = modelMatrix * pos;

    normal = normalize(viewNormalMatrix * normal);
    tangent = normalize(viewNormalMatrix * tangent);
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
