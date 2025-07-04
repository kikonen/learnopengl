#include "AudioPlay.h"

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
    AudioPlay::AudioPlay(
        pool::NodeHandle handle,
        audio::source_id audioSid,
        bool sync) noexcept
        : NodeCommand(handle, 0, false),
        m_audioSid{ audioSid },
        m_sync(sync)
    {
    }

    void AudioPlay::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        audio::Source* source = source = node->getAudioSource(m_audioSid);

        if (!m_started) {
            if (source) {
                source->play();
                m_started = true;
            }
        }

        if (!source) {
            KI_WARN_OUT(fmt::format(
                "CMD: missing_audio: node={}, sid={}, name={}",
                node->getName(), m_audioSid, SID_NAME(m_audioSid)));
        }

        m_finished = source && m_sync ? !source->isPlaying() : true;
    }
}
