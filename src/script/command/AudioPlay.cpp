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
        audio::source_id id,
        bool sync) noexcept
        : NodeCommand(handle, 0, false),
        m_id(id),
        m_sync(sync)
    {
    }

    void AudioPlay::execute(
        const UpdateContext& ctx) noexcept
    {
        audio::Source* source = source = getNode()->getAudioSource(m_id);

        if (!m_started) {
            if (source) {
                source->play();
                m_started = true;
            }
        }

        if (!source) {
            auto* node = getNode();
            KI_WARN_OUT(fmt::format(
                "CMD: missing_audio: node={}, sid={}, name={}",
                getNode()->getName(), m_id, SID_NAME(m_id)));
        }

        m_finished = source && m_sync ? !source->isPlaying() : true;
    }
}
