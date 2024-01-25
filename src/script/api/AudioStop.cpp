#include "AudioStop.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"


namespace script
{
    AudioStop::AudioStop(
        script::command_id afterCommandId,
        ki::node_id nodeId,
        int index) noexcept
        : NodeCommand(afterCommandId, nodeId, 0, false),
        m_index(index)
    {
    }

    void AudioStop::bind(const UpdateContext& ctx, Node* node) noexcept
    {
        NodeCommand::bind(ctx, node);
    }

    void AudioStop::execute(
        const UpdateContext& ctx) noexcept
    {
        if (m_index < 0 || m_index >= ki::MAX_NODE_AUDIO_SOURCE)
        {
            m_finished = true;
            return;
        }

        auto ae = ctx.m_registry->m_audioEngine;
        const auto sourceId = getNode()->m_audioSourceIds[m_index];
        ae->stopSource(sourceId);

        m_finished = true;
    }
}
