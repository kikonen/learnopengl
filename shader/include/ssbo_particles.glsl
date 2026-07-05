const float PARTICLE_MAX_SCALE = 5.0;

#include "include/struct_particle.glsl"

#define _SSBO_PARTICLES
layout (std430, binding = SSBO_PARTICLES) readonly buffer ParticleSSBO {
  Particle u_particles[];
};
