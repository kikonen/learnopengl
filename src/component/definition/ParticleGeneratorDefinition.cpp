#include "ParticleGeneratorDefinition.h"

#include "model/NodeType.h"

#include "particle/ParticleGenerator.h"

std::unique_ptr<particle::ParticleGenerator> ParticleGeneratorDefinition::createParticleGenerator(
    const NodeType* type)
{
    if (!type->m_particleGeneratorDefinition) return nullptr;
    auto generator = std::make_unique<particle::ParticleGenerator>();
    generator->setDefinition(type->m_particleGeneratorDefinition->m_data);
    return generator;
}
