#pragma once

#include <vector>
#include <string>

#include <AL/al.h>

#include "size.h"

namespace audio
{
    struct Sound {
        Sound() = default;
        Sound(Sound&&) noexcept;
        ~Sound();

        // main thread
        void prepare();

        // worker thread
        bool load(std::string_view fullPath);

        audio::sound_id m_id{ 0 };
        ALuint m_bufferId{ 0 };

        int m_sampleRate{ 0 };
        int m_bitDepth{ 0 };

        int m_sampleCount{ 0 };
        double m_lengthInSeconds{ 0 };

        int m_channelCount{ 0 };
        bool m_isMono{ false };
        bool m_isStereo{ false };

        ALenum m_format{ 0 };
        std::vector<ALint> m_data{};
    };
}
