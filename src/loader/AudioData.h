#pragma once

#include <string>
#include <vector>

#include "audio/limits.h"

namespace loader
{
    struct ListenerData {
        bool enabled{ false };
        bool isDefault{ false };

        float gain{ 1.f };
    };

    // AL_MIN_GAIN
    // AL_MAX_GAIN

    // AL_MAX_DISTANCE
    // AL_ROLLOFF_FACTOR
    // AL_REFERENCE_DISTANCE

    // AL_CONE_OUTER_GAIN
    // AL_CONE_INNER_ANGLE
    // AL_CONE_OUTER_ANGL
    struct SourceData {
        bool enabled{ false };

        std::string name;
        std::string path;

        float referenceDistance{ audio::REFERENCE_DISTANCE };
        float maxDistance{ audio::MAX_DISTANCE };
        float rolloffFactor{ audio::ROLLOFF_FACTOR };

        float minGain{ audio::MIN_GAIN };
        float maxGain{ audio::MAX_GAIN };

        bool looping{ false };

        float pitch{ 1.f };
        float gain{ 1.f };
    };

    struct AudioData {
        ListenerData listener;
        std::vector<SourceData> sources;
    };
}
