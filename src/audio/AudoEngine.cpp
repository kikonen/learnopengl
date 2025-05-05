#include "AudioEngine.h"

#include <unordered_map>

#include <fmt/format.h>

#include "util/thread.h"
#include "util/Log.h"
#include "util/util.h"

#include "al_call.h"

#include "SoundRegistry.h"
#include "Sound.h"
#include "Source.h"
#include "Listener.h"

namespace
{
    static audio::AudioEngine* s_engine{ nullptr };
}

namespace audio
{
    void AudioEngine::init() noexcept
    {
        assert(!s_engine);
        s_engine = new AudioEngine();
    }

    void AudioEngine::release() noexcept
    {
        auto* s = s_engine;
        s_engine = nullptr;
        delete s;
    }

    AudioEngine& AudioEngine::get() noexcept
    {
        assert(s_engine);
        return *s_engine;
    }
}

namespace audio
{
    struct Sound;

    AudioEngine::AudioEngine()
        : m_soundRegistry(std::make_unique<SoundRegistry>())
    {
    }

    AudioEngine::~AudioEngine()
    {
    }

    void AudioEngine::clear()
    {
        ASSERT_WT();

        // TODO KI unregister sources and listeners

        m_activeListenerId = 0;

        // NOTE KI MUST close buffers before closing context
        m_soundRegistry->clear();
    }

    void AudioEngine::shutdown()
    {
        ASSERT_WT();

        // [ALSOFT] (WW) Error generated on context 0x214e0406960, code 0xa004, "Deleting in-use buffer 1"
        // [ALSOFT](WW) 2 Sources not deleted
        // [ALSOFT](WW) 1 Buffer not deleted

        clear();

        if (m_context) {
            alcDestroyContext(m_context);
        }

        if (m_device) {
            alcCloseDevice(m_device);
        }
    }

    void AudioEngine::prepare()
    {
        ASSERT_WT();

        // NOTE KI use default device
        m_device = alcOpenDevice(nullptr);
        if (!m_device)
        {
            KI_CRITICAL("AUDIO_ERROR: Failed to open audio device");
            return;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context)
        {
            KI_CRITICAL("AUDIO_ERROR: Could not create audio context");
            return;
        }

        ALCboolean curr = alcMakeContextCurrent(m_context);
        if (curr != ALC_TRUE)
        {
            KI_CRITICAL("AUDIO_ERROR: Could not make audio context current");
            return;
        }

        // default: AL_INVERSE_DISTANCE_CLAMPED
        // => NOTE KI *NOT* configurable since changing this requires adjusting *all* other params also
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        alDopplerFactor(1.f);
        alSpeedOfSound(343.3f);

        // NOTE KI by default no gain in listener
        alListenerf(AL_GAIN, 0.f);
    }

    void AudioEngine::setActiveListenerId(ki::node_id nodeId)
    {
        m_activeListenerId = nodeId;

        if (!nodeId) {
            // NOTE KI silence audio if no listener; don't stop
            alListenerf(AL_GAIN, 0.f);
        }
    }

    void AudioEngine::prepareSource(audio::Source& source)
    {
        auto* sound = m_soundRegistry->getSound(source.m_soundId);
        if (!sound) return;

        sound->prepare();
        if (!sound->m_bufferId) return;

        source.prepare(sound);
    }

    audio::sound_id AudioEngine::registerSound(std::string_view fullPath)
    {
        return m_soundRegistry->registerSound(fullPath);
    }
}
