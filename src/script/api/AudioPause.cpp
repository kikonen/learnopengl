#include "AudioPause.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"


namespace script
{
    AudioPause::AudioPause(
        ki::node_id nodeId,
        int index) noexcept
        : NodeCommand(nodeId, 0, false),
        m_index(index)
    {
    }

    void AudioPause::execute(
        const UpdateContext& ctx) noexcept
    {
        if (m_index < 0 || m_index >= ki::MAX_NODE_AUDIO_SOURCE)
        {
            m_finished = true;
            return;
        }

        auto ae = ctx.m_registry->m_audioEngine;
        const auto sourceId = getNode()->m_audioSourceIds[m_index];
        ae->pauseSource(sourceId);

        m_finished = true;
    }
}
