#pragma once

#include <vector>

#include "BaseLoader.h"
#include "ParticleData.h"

namespace particle {
    class ParticleGenerator;
}

namespace loader {
    struct MAterialData;

    class ParticleLoader : public BaseLoader
    {
    public:
        ParticleLoader(
            Context ctx);

        void loadParticle(
            const YAML::Node& node,
            ParticleData& data) const;

        std::unique_ptr<particle::ParticleGenerator> createParticle(
            const ParticleData& data,
            const std::vector<MaterialData>& materials) const;
    };
}
