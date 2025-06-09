#include "ParticleStop.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "particle/ParticleGenerator.h"

#include "registry/Registry.h"

namespace script
{
    ParticleStop::ParticleStop(
        pool::NodeHandle handle) noexcept
        : NodeCommand(handle, 0, false)
    {
    }

    void ParticleStop::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* generator = getNode()->m_particleGenerator.get();

        if (!m_started) {
            if (generator) {
                generator->clear();
                m_started = true;
            }
        }

        m_finished = true;
    }
}
