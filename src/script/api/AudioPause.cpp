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
        audio::source_id id) noexcept
        : NodeCommand(handle, 0, false),
        m_id(id)
    {
    }

    void AudioPause::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* source = getNode()->getAudioSource(m_id);

        if (source) {
            source->pause();
        }

        m_finished = true;
    }
}
