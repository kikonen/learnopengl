#define _SSBO_PARTICLES
layout (std430, binding = SSBO_PARTICLES) readonly buffer ParticleSSBO {
  Particle u_particles[];
};
