#pragma once

#include "ki/size.h"

#include "asset/Material.h"

#include "ParticleDefinition.h"

struct PrepareContext;
struct UpdateContext;

namespace particle {
    class ParticleGenerator final
    {
    public:
        ParticleGenerator();

        ~ParticleGenerator();

        void prepareWT();

        void updateWT(const UpdateContext& ctx);

        void setMaterial(const Material& material) {
            m_material = material;
        }

        void setDefinition(const ParticleDefinition& definition)
        {
            m_definition = definition;
        }

    private:
        Material m_material;
        ParticleDefinition m_definition;

        float m_lastTs = -1;
    };
}
