#include "AudioPlay.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"

namespace script
{
    AudioPlay::AudioPlay(
        ki::node_id nodeId,
        int index,
        bool sync) noexcept
        : NodeCommand(nodeId, 0, false),
        m_index(index),
        m_sync(sync)
    {
    }

    void AudioPlay::execute(
        const UpdateContext& ctx) noexcept
    {
        if (m_index < 0 || m_index >= ki::MAX_NODE_AUDIO_SOURCE)
        {
            m_finished = true;
            return;
        }

        auto& ae = audio::AudioEngine::get();
        const auto sourceId = getNode()->m_audioSourceIds[m_index];
        if (!m_started) {
            ae.playSource(sourceId);
            m_started = true;
        }
        m_finished = m_sync ? !ae.isPlaying(sourceId) : true;
    }
}
