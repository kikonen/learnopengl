#include "AudioStop.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/Source.h"

#include "registry/Registry.h"


namespace script
{
    AudioStop::AudioStop(
        pool::NodeHandle handle,
        unsigned int id) noexcept
        : NodeCommand(handle, 0, false),
        m_id(id)
    {
    }

    void AudioStop::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* source = getNode()->getAudioSource(m_id);

        if (source) {
            source->stop();
        }

        m_finished = true;
    }
}
