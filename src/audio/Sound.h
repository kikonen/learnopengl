#pragma once

#include <vector>
#include <string>

#include <AL/al.h>

#include "size.h"

namespace audio
{
    struct Sound {
        Sound() = default;
        Sound(std::string_view fullPath);
        ~Sound();

        // main thread
        void prepare();

        // worker thread
        bool load();

        audio::sound_id m_id{ 0 };

        std::string m_fullPath;

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
