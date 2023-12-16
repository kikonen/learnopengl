#include "AudioPause.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"


AudioPause::AudioPause(
    ki::command_id afterCommandId,
    ki::object_id nodeId,
    int index) noexcept
    : NodeCommand(afterCommandId, nodeId, 0, false),
    m_index(index)
{
}

void AudioPause::bind(const UpdateContext& ctx, Node* node) noexcept
{
    NodeCommand::bind(ctx, node);
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
    const auto sourceId = m_node->m_audioSourceIds[m_index];
    ae->pauseSource(sourceId);

    m_finished = true;
}
