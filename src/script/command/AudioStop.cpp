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
        audio::source_id audioSid) noexcept
        : NodeCommand(handle, 0, false),
        m_audioSid{ audioSid }
    {
    }

    void AudioStop::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* source = getNode()->getAudioSource(m_audioSid);

        if (source) {
            source->stop();
        }

        if (!source) {
            KI_WARN_OUT(fmt::format(
                "CMD: missing_audio: node={}, sid={}, name={}",
                getNode()->getName(), m_audioSid, SID_NAME(m_audioSid)));
        }

        m_finished = true;
    }
}
