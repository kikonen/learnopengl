#pragma once

#include <string>


namespace loader
{
    struct ListenerData {
        bool enabled{ false };
        bool isDefault{ false };

        float gain{ 1.f };
    };

    struct SourceData {
        bool enabled{ false };
        bool isAutoPlay{ false };

        std::string path;

        bool looping{ false };

        float pitch{ 1.f };
        float gain{ 1.f };

        // AL_MIN_GAIN
        // AL_MAX_GAIN

        // AL_MAX_DISTANCE
        // AL_ROLLOFF_FACTOR
        // AL_REFERENCE_DISTANCE

        // AL_CONE_OUTER_GAIN
        // AL_CONE_INNER_ANGLE
        // AL_CONE_OUTER_ANGL
    };

    struct AudioData {
        ListenerData listener;
        SourceData source;
    };
}
