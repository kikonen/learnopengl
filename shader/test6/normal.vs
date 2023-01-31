#version 460 core

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  vec3 normal;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  const int materialIndex = entity.materialIndex;
  const mat3 normalMatrix = transpose(inverse(mat3(u_viewMatrix * entity.modelMatrix)));

  vec4 worldPos;

  if ((entity.flags & ENTITY_BILLBOARD_BIT) == ENTITY_BILLBOARD_BIT) {
    // https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
    // - "ogl" approach
    vec3 entityPos = vec3(entity.modelMatrix[3]);
    vec3 entityScale = vec3(entity.modelMatrix[0][0],
                            entity.modelMatrix[1][1],
                            entity.modelMatrix[2][2]);

    worldPos = vec4(entityPos
                    + u_viewRight * a_pos.x * entityScale.x
                    + u_viewUp * a_pos.y * entityScale.y,
                    1.0);
  } else {
    worldPos = entity.modelMatrix * a_pos;
  }

  gl_Position = u_viewMatrix * worldPos;

  vs_out.normal = normalize(normalMatrix * a_normal);
}
