#pragma once

#include <memory>

#include "particle/ParticleDefinition.h"

namespace particle
{
    class ParticleGenerator;
}

class NodeType;

struct ParticleGeneratorDefinition
{
    particle::ParticleDefinition m_data;

    static std::unique_ptr<particle::ParticleGenerator> createParticleGenerator(
        const NodeType* type);
};
