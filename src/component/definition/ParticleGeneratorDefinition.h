#pragma once

#include <memory>

#include "particle/ParticleDefinition.h"

namespace particle
{
    class ParticleGenerator;
}

namespace model
{
    class NodeType;
}

struct ParticleGeneratorDefinition
{
    particle::ParticleDefinition m_data;

    static std::unique_ptr<particle::ParticleGenerator> createParticleGenerator(
        const model::NodeType* type);
};
