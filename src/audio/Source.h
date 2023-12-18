#pragma once

#include <glm/glm.hpp>

#include "AL/al.h"

#include "size.h"
#include "limits.h"


namespace audio
{
    struct Sound;

    struct Source {
        Source();
        ~Source();

        void prepare(const Sound* sound);

        void update();
        void updatePos();

        void play();
        void stop();
        void pause();

        bool isPlaying();
        bool isPaused();

        audio::source_id m_id;

        ALuint m_sourceId{ 0 };

        audio::sound_id m_soundId{ 0 };

        float m_referenceDistance{ audio::REFERENCE_DISTANCE };
        float m_maxDistance{ audio::MAX_DISTANCE };
        float m_rolloffFactor{ audio::ROLLOFF_FACTOR };

        float m_minGain{ audio::MIN_GAIN };
        float m_maxGain{ audio::MAX_GAIN };

        bool m_looping{ false };

        float m_pitch{ 1.f };
        float m_gain{ 1.f };

        // AL_MIN_GAIN
        // AL_MAX_GAIN

        // AL_MAX_DISTANCE
        // AL_ROLLOFF_FACTOR
        // AL_REFERENCE_DISTANCE

        // AL_CONE_OUTER_GAIN
        // AL_CONE_INNER_ANGLE
        // AL_CONE_OUTER_ANGL

        glm::vec3 m_pos{ 0.f };
        glm::vec3 m_vel{ 0.f };
        glm::vec3 m_dir{ 0.f };
    };
}
