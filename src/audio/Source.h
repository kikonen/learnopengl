#pragma once

#include <glm/glm.hpp>

#include "al_call.h"

#include "ki/size.h"

#include "size.h"
#include "limits.h"

struct NodeState;

namespace audio
{
    struct Sound;

    // AL_MIN_GAIN
    // AL_MAX_GAIN
    // AL_MAX_DISTANCE
    // AL_ROLLOFF_FACTOR
    // AL_REFERENCE_DISTANCE
    // AL_CONE_OUTER_GAIN
    // AL_CONE_INNER_ANGLE
    // AL_CONE_OUTER_ANGL
    struct Source {
        Source() {}
        Source(Source& o) = delete;
        Source(const Source& o) = delete;
        Source(Source&&) noexcept;
        ~Source();

        Source& operator=(Source& o) = delete;
        Source& operator=(Source&& o) noexcept;

        void prepare(const Sound* sound);

        void update(const NodeState& state);

        void play() const;
        void stop() const;
        void pause() const;
        void toggle(bool play) const;

        bool isPlaying() const;
        bool isPaused() const;

        audio::source_id m_id{ 0 };
        audio::sound_id m_soundId{ 0 };

        float m_referenceDistance{ audio::REFERENCE_DISTANCE };
        float m_maxDistance{ audio::MAX_DISTANCE };
        float m_rolloffFactor{ audio::ROLLOFF_FACTOR };

        float m_minGain{ audio::MIN_GAIN };
        float m_maxGain{ audio::MAX_GAIN };

        bool m_looping{ false };

        float m_pitch{ 1.f };
        float m_gain{ 1.f };

        ALuint m_sourceId{ 0 };

        ki::level_id m_matrixLevel{ 0 };
    };
}
