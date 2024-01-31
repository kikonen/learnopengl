#include "AudioStop.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"


namespace script
{
    AudioStop::AudioStop(
        ki::node_id nodeId,
        int index) noexcept
        : NodeCommand(nodeId, 0, false),
        m_index(index)
    {
    }

    void AudioStop::execute(
        const UpdateContext& ctx) noexcept
    {
        if (m_index < 0 || m_index >= ki::MAX_NODE_AUDIO_SOURCE)
        {
            m_finished = true;
            return;
        }

        auto& ae = audio::AudioEngine::get();
        const auto sourceId = getNode()->m_audioSourceIds[m_index];
        ae.stopSource(sourceId);

        m_finished = true;
    }
}
