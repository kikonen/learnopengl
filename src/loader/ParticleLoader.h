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

        void loadParticles(
            const loader::DocNode& node,
            std::vector<ParticleData>& particles,
            Loaders& loaders) const;

        void loadParticle(
            const loader::DocNode& node,
            ParticleData& data,
            const std::unordered_map<std::string, const loader::DocNode*>& idToParticle,
            Loaders& loaders) const;

        std::unique_ptr<ParticleGeneratorDefinition> createDefinition(
            const ParticleData& data) const;
    };
}
