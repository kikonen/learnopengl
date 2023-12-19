#include "Source.h"

#include <fmt/format.h>
#include "al_call.h"

#include "util/Log.h"

#include "Sound.h"

namespace {
}

namespace audio
{
    Source::Source()
    {
    }

    Source::Source(Source&& b) noexcept
        : m_id{ b.m_id },
        m_sourceId{ b.m_sourceId },
        m_soundId{ b.m_soundId },
        m_referenceDistance{ b.m_referenceDistance },
        m_maxDistance{ b.m_maxDistance },
        m_rolloffFactor{ b.m_rolloffFactor },
        m_minGain{ b.m_minGain  },
        m_maxGain{ b.m_maxGain },
        m_looping{ b.m_looping },
        m_pitch{ b.m_pitch },
        m_gain{ b.m_gain },
        m_pos{ b.m_pos },
        m_vel{ b.m_vel },
        m_dir{ b.m_dir }
    {
        // NOTE KI b is moved now
        b.m_sourceId = 0;
    }

    Source::~Source()
    {
        if (m_sourceId) {
            alDeleteSources(1, &m_sourceId);
        }
    }

    void Source::prepare(const Sound* sound) {
        if (m_sourceId) return;

        m_soundId = sound->m_id;

        alGenSources(1, &m_sourceId);
        alSourcei(m_sourceId, AL_BUFFER, sound->m_bufferId);

        {
            ALfloat referenceDistance;
            ALfloat maxDistance;
            ALfloat rolloffFactor;
            ALfloat minGain;
            ALfloat maxGain;

            alGetSourcef(m_sourceId, AL_REFERENCE_DISTANCE, &referenceDistance);
            alGetSourcef(m_sourceId, AL_MAX_DISTANCE, &maxDistance);
            alGetSourcef(m_sourceId, AL_ROLLOFF_FACTOR, &rolloffFactor);
            alGetSourcef(m_sourceId, AL_MIN_GAIN, &minGain);
            alGetSourcef(m_sourceId, AL_MAX_GAIN, &maxGain);

            KI_INFO_OUT(
                fmt::format(
                    "SOURCE: id={}, soundId={}, referenceDistance={}, maxDistance={}, rolloffFactor={}, minGain={}, maxGain={}",
                    m_id, m_soundId, referenceDistance, maxDistance, rolloffFactor, minGain, maxGain));
        }

        //// NOTE KI ensure defaults are in place
        //update();
    }

    void Source::update() {
        if (!m_sourceId) return;

        alSourcef(m_sourceId, AL_REFERENCE_DISTANCE, m_referenceDistance);
        alSourcef(m_sourceId, AL_MAX_DISTANCE, m_maxDistance);
        alSourcef(m_sourceId, AL_ROLLOFF_FACTOR, m_rolloffFactor);

        alSourcef(m_sourceId, AL_MIN_GAIN, m_minGain);
        alSourcef(m_sourceId, AL_MAX_GAIN, m_maxGain);

        alSourcei(m_sourceId, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
        alSourcef(m_sourceId, AL_PITCH, m_pitch);
        alSourcef(m_sourceId, AL_GAIN, m_gain);

        updatePos();
    }

    void Source::updatePos() {
        alSource3f(m_sourceId, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(m_sourceId, AL_VELOCITY, m_vel.x, m_vel.y, m_vel.z);
        alSource3f(m_sourceId, AL_DIRECTION, m_dir.x, m_dir.y, m_dir.z);
    }

    void Source::play() const
    {
        if (isPlaying()) return;
        alSourcePlay(m_sourceId);
    }

    void Source::stop() const
    {
        alSourceStop(m_sourceId);
    }

    void Source::pause() const
    {
        alSourcePause(m_sourceId);
    }

    bool Source::isPlaying() const
    {
        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }

    bool Source::isPaused() const
    {
        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
}
