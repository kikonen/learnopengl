#pragma once

#include <memory>

#include "util/Ref.h"

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

    static util::Ref<particle::ParticleGenerator> createParticleGenerator(
        const model::NodeType* type);
};
