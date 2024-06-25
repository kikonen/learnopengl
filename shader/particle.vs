#version 460 core

#include struct_particle.glsl

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_clip_plane.glsl

#include ssbo_particles.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat vec2 spriteCoord;
  flat vec2 spriteSize;
  vec3 viewPos;

  flat vec4 diffuse;
  flat uvec2 diffuseTex;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Particle particle;

#include fn_decode.glsl
#include fn_calculate_clipping.glsl

void main() {
  const uint particleIndex = gl_BaseInstance + gl_InstanceID;
  particle = u_particles[particleIndex];

  const uint materialIndex = particle.u_materialIndex;

  const vec4 worldPos = vec4(particle.u_pos_scale.xyz, 1.0);
  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;

  gl_Position = u_projectedMatrix * worldPos;

  const float particleScale = particle.u_pos_scale.a / gl_Position.w;
  gl_PointSize = 2000 * particleScale;

  vs_out.diffuse = u_materials[materialIndex].diffuse;
  vs_out.diffuseTex = u_materials[materialIndex].diffuseTex;

  const uint spriteIndex = particle.u_spriteIndex;

  const uint spritesX = u_materials[materialIndex].spritesX;
  const uint spritesY = u_materials[materialIndex].spritesY;

  const float tx = 1.0 / spritesX;
  const float ty = 1.0 / spritesY;

  const uint sx = spriteIndex % spritesX;
  const uint sy = spriteIndex / spritesX;

  vs_out.spriteCoord.x = sx * tx;
  vs_out.spriteCoord.y = 1.0 - (sy + 1) * ty;

  vs_out.spriteSize.x = tx;
  vs_out.spriteSize.y = ty;

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  calculateClipping(worldPos);
}
