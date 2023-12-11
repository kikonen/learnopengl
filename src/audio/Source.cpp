#include "Source.h"

#include <mutex>


namespace {
    audio::source_id idBase{ 0 };

    std::mutex id_lock{};

    audio::source_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Source::Source(audio::audio_id audioId)
        : m_id{ nextID() },
        m_audioId{ audioId }
    {}

    Source::~Source()
    {}
}
