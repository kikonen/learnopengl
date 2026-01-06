#include "Source.h"

#include <algorithm>

#include <fmt/format.h>
#include "al_call.h"

#include "util/Log.h"

#include "model/NodeState.h"

#include "Sound.h"

namespace {
}

namespace audio
{
    Source::Source(Source&& o) noexcept
    {
        swap(o);
    }

    Source::~Source()
    {
        if (m_sourceId) {
            alSourceStop(m_sourceId);
            alDeleteSources(1, &m_sourceId);
        }
    }

    Source& Source::operator=(Source&& o) noexcept
    {
        Source tmp(std::move(o));
        swap(tmp);
        return *this;
    }

    void Source::swap(Source& o) noexcept
    {
        std::swap(m_id, o.m_id);
        std::swap(m_sourceId, o.m_sourceId);
        std::swap(m_soundId, o.m_soundId);
        std::swap(m_referenceDistance, o.m_referenceDistance);
        std::swap(m_maxDistance, o.m_maxDistance);
        std::swap(m_rolloffFactor, o.m_rolloffFactor);
        std::swap(m_minGain, o.m_minGain);
        std::swap(m_maxGain, o.m_maxGain);
        std::swap(m_looping, o.m_looping);
        std::swap(m_pitch, o.m_pitch);
        std::swap(m_gain, o.m_gain);
    }

    void Source::prepare(const Sound* sound) {
        if (m_sourceId) return;

        m_soundId = sound->m_id;

        alGenSources(1, &m_sourceId);
        alSourcei(m_sourceId, AL_BUFFER, sound->m_bufferId);

        {
            alSourcef(m_sourceId, AL_REFERENCE_DISTANCE, m_referenceDistance);
            alSourcef(m_sourceId, AL_MAX_DISTANCE, m_maxDistance);
            alSourcef(m_sourceId, AL_ROLLOFF_FACTOR, m_rolloffFactor);

            alSourcef(m_sourceId, AL_MIN_GAIN, m_minGain);
            alSourcef(m_sourceId, AL_MAX_GAIN, m_maxGain);

            alSourcei(m_sourceId, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
            alSourcef(m_sourceId, AL_PITCH, m_pitch);
            alSourcef(m_sourceId, AL_GAIN, m_gain);
        }

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
    }

    void Source::update(const model::NodeState& state)
    {
        if (!m_sourceId) return;

        const auto level = state.m_matrixLevel;
        if (m_matrixLevel == level) return;
        m_matrixLevel = level;

        const auto& pos = state.getWorldPosition();
        const auto& dir = glm::normalize(state.getViewFront());
        const auto& vel = glm::vec3{ 0.f };

        {
            alSource3f(m_sourceId, AL_POSITION, pos.x, pos.y, pos.z);
            alSource3f(m_sourceId, AL_VELOCITY, vel.x, vel.y, vel.z);
            alSource3f(m_sourceId, AL_DIRECTION, dir.x, dir.y, dir.z);
        }
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

    void Source::toggle(bool isPlay) const
    {
        if (isPlay) {
            play();
        }
        else {
            stop();
        }
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
