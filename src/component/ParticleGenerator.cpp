#include "ParticleGenerator.h"

#include "model/Particle.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "scene/ParticleSystem.h"


void ParticleGenerator::update(const UpdateContext& ctx)
{
    float x = m_definition.particlesPerSec * ctx.m_clock.elapsedSecs;

    //if ()
//    definition.
    Particle particle;
    particle.pos = { 10, 10, 10 };
    particle.dir = { 0, 1, 0 };
    particle.velocity = 2;
    particle.lifetime = 5;

    m_system->addParticle(particle);
}
