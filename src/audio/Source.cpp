#include "Source.h"

#include <mutex>

#include "al_call.h"

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

        // NOTE KI ensure defaults are in place
        update();
    }

    void Source::update() {
        if (!m_sourceId) return;

        alSourcei(m_sourceId, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
        alSourcef(m_sourceId, AL_PITCH, m_pitch);
        alSourcef(m_sourceId, AL_GAIN, m_gain);

        alSource3f(m_sourceId, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(m_sourceId, AL_VELOCITY, m_vel.x, m_vel.y, m_vel.z);
        alSource3f(m_sourceId, AL_DIRECTION, m_dir.x, m_dir.y, m_dir.z);
    }

    void Source::play()
    {
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
