#pragma once

#include <string>

#include <AL/al.h>

#include "size.h"

namespace audio
{
    struct Audio {
        Audio(std::string_view path);
        ~Audio();

        void load(const std::string_view assetsPath);

        audio::audio_id m_id;

        std::string m_path;

        int m_sampleRate{ 0 };
        int m_bitDepth{ 0 };

        int m_sampleCount{ 0 };
        double m_lengthInSeconds{ 0 };

        int m_channelCount{ 0 };
        bool m_isMono{ false };
        bool m_isStereo{ false };

    };
}
