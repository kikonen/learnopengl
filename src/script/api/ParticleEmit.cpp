#include "ParticleEmit.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "particle/ParticleGenerator.h"


#include "registry/Registry.h"

namespace script
{
    ParticleEmit::ParticleEmit(
        pool::NodeHandle handle,
        float count,
        bool sync) noexcept
        : NodeCommand(handle, 0, false),
        m_count(count),
        m_sync(sync)
    {
    }

    void ParticleEmit::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* generator = getNode()->m_particleGenerator.get();

        if (!m_started) {
            if (generator) {
                generator->emit(m_count);
                m_started = true;
            }
        }

        m_finished = generator && m_sync ? !generator->isEmitting() : true;
    }
}
