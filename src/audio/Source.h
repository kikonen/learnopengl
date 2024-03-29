#pragma once

#include <glm/glm.hpp>

#include "AL/al.h"

#include "ki/size.h"

#include "size.h"
#include "limits.h"

#include "pool/NodeHandle.h"

struct Snapshot;

namespace audio
{
    struct Sound;

    struct Source {
        Source() {}
        Source(Source& o) = delete;
        Source(const Source& o) = delete;
        Source(Source&&) noexcept;
        ~Source();

        Source& operator=(Source& o) = delete;
        Source& operator=(Source&& o) noexcept;

        inline bool isReady() const { return m_matrixLevel > -1; }

        void prepare(const Sound* sound);

        void updateFromSnapshot(const Snapshot& snapshot);

        void update();
        void updatePos();

        void play() const;
        void stop() const;
        void pause() const;

        bool isPlaying() const;
        bool isPaused() const;

        audio::source_id m_id{ 0 };

        ALuint m_sourceId{ 0 };

        audio::sound_id m_soundId{ 0 };

        bool m_autoPlay{ false };

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

        pool::NodeHandle m_nodeHandle{};

        ki::level_id m_matrixLevel{ 0 };
    };
}
