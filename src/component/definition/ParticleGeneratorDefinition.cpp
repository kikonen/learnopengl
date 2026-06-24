#include "ParticleGeneratorDefinition.h"

#include "model/NodeType.h"

#include "particle/ParticleGenerator.h"

util::Ref<particle::ParticleGenerator> ParticleGeneratorDefinition::createParticleGenerator(
    const model::NodeType* type)
{
    if (!type->m_particleGeneratorDefinition) return nullptr;
    auto generator = util::Ref<particle::ParticleGenerator>::create();
    generator->setDefinition(type->m_particleGeneratorDefinition->m_data);
    return generator;
}
