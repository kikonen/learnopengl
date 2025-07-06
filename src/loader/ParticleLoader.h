#pragma once

#include <vector>

#include "BaseLoader.h"
#include "ParticleData.h"

struct ParticleGeneratorDefinition;

namespace loader {
    class Loaders;

    class ParticleLoader : public BaseLoader
    {
    public:
        ParticleLoader(
            std::shared_ptr<Context> ctx);

        void loadParticle(
            const loader::DocNode& node,
            ParticleData& data,
            Loaders& loaders) const;

        std::unique_ptr<ParticleGeneratorDefinition> createDefinition(
            const ParticleData& data) const;
    };
}
