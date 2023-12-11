#pragma once

#include <glm/glm.hpp>

#include "size.h"

namespace audio
{
    struct Source {
        Source(audio::audio_id audioId);
        ~Source();

        audio::source_id m_id;
        audio::audio_id m_audioId{ 0 };

        glm::vec3 m_pos{ 0.f };
    };
}
