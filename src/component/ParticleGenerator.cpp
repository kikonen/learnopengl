#include "ParticleGenerator.h"

#include "asset/Assets.h"
#include "scene/ParticleSystem.h"

#include "model/Particle.h"

#include "scene/RenderContext.h"

ParticleGenerator::ParticleGenerator(
    const ParticleDefinition definition)
    : definition(definition)
{
}

void ParticleGenerator::update(const RenderContext& ctx)
{
    float x = definition.particlesPerSec * ctx.m_clock.elapsedSecs;

    //if ()
//    definition.
    Particle particle;
    particle.pos = { 10, 10, 10 };
    particle.dir = { 0, 1, 0 };
    particle.velocity = 2;
    particle.lifetime = 5;

    system->addParticle(particle);
}
