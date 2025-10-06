#include "AudioSystem.h"

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
    static audio::AudioSystem* s_system{ nullptr };
}

namespace audio
{
    void AudioSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new AudioSystem();
    }

    void AudioSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    AudioSystem& AudioSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace audio
{
    struct Sound;

    AudioSystem::AudioSystem()
        : m_soundRegistry(std::make_unique<SoundRegistry>())
    {
    }

    AudioSystem::~AudioSystem()
    {
    }

    void AudioSystem::clear()
    {
        //ASSERT_WT();

        // NOTE KI MUST close buffers before closing context
        m_soundRegistry->clear();
    }

    void AudioSystem::shutdown()
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

    void AudioSystem::prepare()
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

    void AudioSystem::prepareSource(audio::Source& source)
    {
        auto* sound = m_soundRegistry->getSound(source.m_soundId);
        if (!sound) return;

        sound->prepare();
        if (!sound->m_bufferId) return;

        source.prepare(sound);
    }

    audio::sound_id AudioSystem::registerSound(std::string_view fullPath)
    {
        return m_soundRegistry->registerSound(fullPath);
    }
}
