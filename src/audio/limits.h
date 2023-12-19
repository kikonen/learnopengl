#pragma once

namespace audio
{
    // OpenAL:
    // referenceDistance = 1
    // maxDistance = 3.4028235e+38
    // rolloffFactor = 1
    // minGain=1
    // maxGain=-107374180

    // NOTE KI bigger = attenuation starts later, smaller = attenuation starts earlier
    constexpr float REFERENCE_DISTANCE{ 1.f };

    // NOTE KI bigger = faster decay, smaller = slower decay
    constexpr float ROLLOFF_FACTOR{ 1.f };

    // NOTE KI MAX_DISTANCE distance after gain decay does NOT decrease anymore
    // => AVOID setting this
    constexpr float MAX_DISTANCE{ std::numeric_limits<float>::max() };

    constexpr float MIN_GAIN{ 0.f };
    constexpr float MAX_GAIN{ 1.f };
}
