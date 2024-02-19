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
  vec2 texCoord;
  vec3 viewPos;

  flat vec4 diffuse;
  flat uvec2 diffuseTex;
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

  vs_out.diffuse = u_materials[materialIndex].diffuse;
  vs_out.diffuseTex = u_materials[materialIndex].diffuseTex;

  const uint spriteIndex = particle.u_spriteIndex;

  const uint spritesX = u_materials[materialIndex].spritesX;
  const uint spritesY = u_materials[materialIndex].spritesY;

  const float tx = 1.0 / spritesX;
  const float ty = 1.0 / spritesY;

  const uint sx = spriteIndex % spritesX;
  const uint sy = spriteIndex / spritesX;

  if (a_texCoord.x == 0.0) {
    vs_out.texCoord.x = sx * tx;
  } else {
    vs_out.texCoord.x = (sx + 1) * tx;
  }

  if (a_texCoord.y == 0.0) {
    vs_out.texCoord.y = 1.0 - (sy + 1) * ty;
  } else {
    vs_out.texCoord.y = 1.0 - sy * ty;
  }

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  calculateClipping(worldPos);
}
