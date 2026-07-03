#version 460 core

#include "include/tbo_materials.glsl"

#include "include/ssbo_particles.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_clip_planes.glsl"

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

const float MAX_SCALE = 5.0;

Particle particle;
ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"

#include "include/fn_calculate_clipping.glsl"

void main() {
  const uint particleIndex = gl_BaseInstance + gl_InstanceID;
  particle = u_particles[particleIndex];

  const uint msp = particle.u_msp;
  const uint materialIndex = (msp >> 16);
  const float scale = (float((msp >> 8) & 255) / 255.0) * MAX_SCALE;
  const uint spriteIndex = msp & 255;

  const vec3 pos = vec3(particle.u_x, particle.u_y, particle.u_z);
  const vec4 worldPos = vec4(pos, 1.0);
  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;

  gl_Position = u_projectedMatrix * worldPos;

  const float particleScale = scale / gl_Position.w;
  gl_PointSize = 2000 * particleScale;

  fillMaterialSpriteTBO(materialIndex);
  uvec2 diffuseTex = readMaterialDiffuseTexTBO(materialIndex);

  vs_out.diffuse = material.diffuse;
  vs_out.diffuseTex = diffuseTex;

  const uint spritesX = material.spritesX;
  const uint spritesY = material.spritesY;

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
