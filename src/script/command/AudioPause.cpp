#include "AudioPause.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/Source.h"

#include "registry/Registry.h"


namespace script
{
    AudioPause::AudioPause(
        pool::NodeHandle handle,
        audio::source_id audioSid) noexcept
        : NodeCommand(handle, 0, false),
        m_audioSid{ audioSid }
    {
    }

    void AudioPause::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        auto* source = node->getAudioSource(m_audioSid);

        if (source) {
            source->pause();
        }

        m_finished = true;
    }
}
