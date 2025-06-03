#pragma once

#include <memory>

#include "ki/size.h"

#include "util/Random.h"

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

        const Material& getMaterial() {
            return m_material;
        }

        Material& modifyMaterial() {
            return m_material;
        }

        void setDefinition(const ParticleDefinition& definition)
        {
            m_definition = definition;
        }

        void emit(float count)
        {
            m_requestedCount += count;
        }

        bool isEmitting() const noexcept
        {
            return m_requestedCount > 0.f;
        }

        void clear()
        {
            m_requestedCount = 0.f;
            m_pendingCount = 0.f;
        }

    private:
        Material m_material;
        ParticleDefinition m_definition;

        float m_lastTs = -1;

        float m_requestedCount{ 0.f };
        float m_pendingCount{ 0.f };

        std::unique_ptr<util::Random> m_random;
    };
}
