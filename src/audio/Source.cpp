#include "Source.h"

#include <mutex>

#include <fmt/format.h>
#include "al_call.h"

#include "util/Log.h"

#include "Sound.h"

namespace {
    audio::source_id idBase{ 0 };

    std::mutex id_lock{};

    audio::source_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Source::Source()
        : m_id{ nextID() }
    {
    }

    Source::~Source()
    {
        if (m_sourceId) {
            alDeleteSources(1, &m_sourceId);
        }
    }

    void Source::prepare(const Sound* sound) {
        if (!sound) return;

        m_soundId = sound->m_id;
        if (!sound->m_bufferId) return;

        alGenSources(1, &m_sourceId);
        alSourcei(m_sourceId, AL_BUFFER, sound->m_bufferId);

        ALfloat referenceDistance;
        ALfloat maxDistance;
        ALfloat rolloffFactor;
        ALfloat minGain;
        ALfloat maxGain;

        alGetSourcef(m_sourceId, AL_REFERENCE_DISTANCE, &referenceDistance);
        alGetSourcef(m_sourceId, AL_MAX_DISTANCE, &maxDistance);
        alGetSourcef(m_sourceId, AL_ROLLOFF_FACTOR, &rolloffFactor);
        alGetSourcef(m_sourceId, AL_MIN_GAIN, &minGain);
        alGetSourcef(m_sourceId, AL_MAX_GAIN, &minGain);

        KI_INFO_OUT(
            fmt::format(
            "SOURCE: id={}, referenceDistance={}, maxDistance={}, rolloffFactor={}, minGain={}, maxGain={}",
            m_id, referenceDistance, maxDistance, rolloffFactor, minGain, maxGain));

        // NOTE KI ensure defaults are in place
        update();
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

    void Source::play()
    {
        if (isPlaying()) return;
        alSourcePlay(m_sourceId);
    }

    void Source::stop()
    {
        alSourceStop(m_sourceId);
    }

    void Source::pause()
    {
        alSourcePause(m_sourceId);
    }

    bool Source::isPlaying()
    {
        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }

    bool Source::isPaused()
    {
        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
}
