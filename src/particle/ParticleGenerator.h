#pragma once

#include "ki/size.h"

#include "material/Material.h"

#include "ParticleDefinition.h"

struct PrepareContext;
struct UpdateContext;

class Node;

namespace particle {
    class ParticleGenerator final
    {
    public:
        ParticleGenerator();

        ~ParticleGenerator();

        void prepareWT();

        void updateWT(
            const UpdateContext& ctx,
            Node& node);

        void setMaterial(const Material& material) {
            m_material = material;
        }

        Material& getMaterial() {
            return m_material;
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
