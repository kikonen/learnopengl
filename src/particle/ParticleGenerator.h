#pragma once

#include <memory>

#include "ki/size.h"

#include "util/Random.h"

#include "ParticleDefinition.h"


namespace model
{
    class Node;
}

struct PrepareContext;
struct UpdateContext;

namespace particle {
    class ParticleGenerator final
    {
    public:
        ParticleGenerator();

        ~ParticleGenerator();

        void prepareWT();

        void updateWT(
            const UpdateContext& ctx,
            model::Node& node);

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
        ParticleDefinition m_definition;

        int m_materialIndex{ -1 };

        float m_lastTs = -1;

        float m_requestedCount{ 0.f };
        float m_pendingCount{ 0.f };

        std::unique_ptr<util::Random> m_random;
    };
}
