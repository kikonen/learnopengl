#pragma once

#include <vector>

#include "BaseLoader.h"
#include "ParticleData.h"

namespace particle {
    class ParticleGenerator;
}

namespace loader {
    class Loaders;
    struct MAterialData;

    class ParticleLoader : public BaseLoader
    {
    public:
        ParticleLoader(
            std::shared_ptr<Context> ctx);

        void loadParticle(
            const loader::DocNode& node,
            ParticleData& data,
            Loaders& loaders) const;

        std::unique_ptr<particle::ParticleDefinition> createDefinition(
            const ParticleData& data) const;
    };
}
