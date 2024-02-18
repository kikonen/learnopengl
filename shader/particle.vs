#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_particle.glsl
#include struct_material.glsl
#include struct_clip_plane.glsl

#include ssbo_particles.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 worldPos;
  vec2 texCoord;
  vec3 viewPos;

  flat uint materialIndex;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Particle particle;

#include fn_calculate_clipping.glsl

void main() {
  const uint particleIndex = gl_BaseInstance + gl_InstanceID;
  particle = u_particles[particleIndex];

  const int materialIndex = particle.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  // always billboard
  {
    vec3 particlePos = particle.u_pos_scale.xyz;
    float particleScale = particle.u_pos_scale.a;

    worldPos = vec4(particlePos
                    + u_viewRight * a_pos.x * particleScale
                    + UP * a_pos.y * particleScale,
                    1.0);
  }

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;

  const uvec2 si = particle.u_spriteIndex;
  const float tx = u_materials[materialIndex].tilingX;
  const float ty = u_materials[materialIndex].tilingY;

  if (a_texCoord.x == 0.0) {
    vs_out.texCoord.x = si.x * tx;
  } else {
    vs_out.texCoord.x = (si.x + 1) * tx;
  }

  if (a_texCoord.y == 0.0) {
    vs_out.texCoord.y = 1.0 - (si.y + 1) * ty;
  } else {
    vs_out.texCoord.y = 1.0 - si.y * ty;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  calculateClipping(worldPos);
}
