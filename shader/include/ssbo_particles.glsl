const float PARTICLE_MAX_SCALE = 5.0;

#include "include/struct_particle.glsl"

#define _SSBO_PARTICLES
layout (std430, binding = SSBO_PARTICLES) readonly buffer ParticleSSBO {
  Particle u_particles[];
};

uint unpackParticleIndex(uint msp) {
  return (msp >> 16);
}

float unpackParticleScale(uint msp) {
  return (float((msp >> 8) & 255) / 255.0) * PARTICLE_MAX_SCALE;
}

uint unpackParticleSpriteIndex(uint msp) {
  return msp & 255;;
}
