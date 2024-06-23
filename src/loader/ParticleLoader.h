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
            Context ctx);

        void loadParticle(
            const loader::DocNode& node,
            ParticleData& data,
            Loaders& loaders) const;

        std::unique_ptr<particle::ParticleGenerator> createParticle(
            const ParticleData& data) const;
    };
}
