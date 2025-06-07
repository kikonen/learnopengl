#pragma once

#include "audio/limits.h"
#include "audio/size.h"

// AL_MIN_GAIN
// AL_MAX_GAIN
// AL_MAX_DISTANCE
// AL_ROLLOFF_FACTOR
// AL_REFERENCE_DISTANCE
// AL_CONE_OUTER_GAIN
// AL_CONE_INNER_ANGLE
// AL_CONE_OUTER_ANGL
struct AudioSourceDefinition
{
    float m_referenceDistance{ audio::REFERENCE_DISTANCE };
    float m_maxDistance{ audio::MAX_DISTANCE };
    float m_rolloffFactor{ audio::ROLLOFF_FACTOR };

    float m_minGain{ audio::MIN_GAIN };
    float m_maxGain{ audio::MAX_GAIN };

    float m_pitch{ 1.f };
    float m_gain{ 1.f };

    audio::source_id m_sourceId{ 0 };
    audio::sound_id m_soundId{ 0 };

    bool m_looping{ false };
};
