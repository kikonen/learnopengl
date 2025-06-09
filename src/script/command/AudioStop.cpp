#include "AudioStop.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "ki/sid.h"
#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/Source.h"

#include "registry/Registry.h"


namespace script
{
    AudioStop::AudioStop(
        pool::NodeHandle handle,
        audio::source_id id) noexcept
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

        if (!source) {
            KI_WARN_OUT(fmt::format(
                "CMD: missing_audio: node={}, sid={}, name={}",
                getNode()->getName(), m_id, SID_NAME(m_id)));
        }

        m_finished = true;
    }
}
