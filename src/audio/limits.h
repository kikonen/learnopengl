#pragma once

namespace audio
{
    // OpenAL:
    // referenceDistance = 1
    // maxDistance = 3.4028235e+38
    // rolloffFactor = 1
    // minGain=1
    // maxGain=-107374180
    constexpr float REFERENCE_DISTANCE{ 1.f };
    constexpr float MAX_DISTANCE{ 100.f };
    constexpr float ROLLOFF_FACTOR{ 1.f };

    constexpr float MIN_GAIN{ 0.f };
    constexpr float MAX_GAIN{ 10.f };
}
