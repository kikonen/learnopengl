#include "AudioEngine.h"

#include <fmt/format.h>

#include "util/Log.h"
#include "util/Util.h"

#include "al_call.h"

#include "SoundRegistry.h"
#include "Sound.h"
#include "Source.h"
#include "Listener.h"

namespace {
}

namespace audio
{
    struct Sound;

    AudioEngine::AudioEngine(const Assets& assets)
        : m_assets(assets),
        m_soundRegistry(std::make_unique<SoundRegistry>())
    {}

    AudioEngine::~AudioEngine()
    {
        for (const auto& it : m_sources) {
            it.second->stop();
        }

        m_listeners.clear();
        m_sources.clear();

        if (m_context) {
            alcDestroyContext(m_context);
        }

        if (m_device) {
            alcCloseDevice(m_device);
        }
    }

    void AudioEngine::prepare()
    {
        // NOTE KI use default device
        m_device = alcOpenDevice(nullptr);
        if (!m_device)
        {
            KI_CRITICAL("ERROR: Failed to open audio device");
            return;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context)
        {
            KI_CRITICAL("ERROR: Could not create audio context");
            return;
        }

        ALCboolean curr = alcMakeContextCurrent(m_context);
        if (curr != ALC_TRUE)
        {
            KI_CRITICAL("ERROR: Could not make audio context current");
            return;
        }

        // default: AL_INVERSE_DISTANCE_CLAMPED
        // alDistanceModel
    }

    void AudioEngine::update(const UpdateContext& ctx)
    {
    }

    void AudioEngine::setActiveListener(audio::listener_id id)
    {
        if (m_activeListenerId == id) return;

        m_activeListenerId = id;

        auto* listener = getListener(m_activeListenerId);

        if (!listener) {
            for (const auto& it : m_sources) {
                it.second->pause();
            }
            return;
        }

        listener->update();

        for (const auto& it : m_sources) {
            if (!it.second->isPaused()) continue;
            it.second->play();
        }
    }

    void AudioEngine::playSource(audio::source_id id)
    {
        if (!m_activeListenerId) return;

        auto* source = getSource(id);
        if (!source) return;

        source->play();
    }

    void AudioEngine::stopSource(audio::source_id id)
    {
        auto* source = getSource(id);
        if (!source) return;

        source->stop();
    }

    void AudioEngine::pauseSource(audio::source_id id)
    {
        auto* source = getSource(id);
        if (!source) return;

        source->pause();
    }

    bool AudioEngine::isPlaying(audio::source_id id)
    {
        auto* source = getSource(id);
        return source && source->isPlaying();
    }

    bool AudioEngine::isPaused(audio::source_id id)
    {
        auto* source = getSource(id);
        return source && source->isPaused();
    }

    audio::listener_id AudioEngine::registerListener()
    {
        auto listener = std::make_unique<Listener>();
        const auto& [it, _]  = m_listeners.insert({ listener->m_id, std::move(listener) });
        return it->first;
    }

    Listener* AudioEngine::getListener(
        audio::listener_id id)
    {
        const auto& it = m_listeners.find(id);
        return it != m_listeners.end() ? it->second.get() : nullptr;
    }

    audio::source_id AudioEngine::registerSource(audio::sound_id soundId)
    {
        auto* sound = m_soundRegistry->getSound(soundId);
        if (!sound) return 0;

        sound->prepare();

        auto source = std::make_unique<Source>();
        source->prepare(sound);

        if (!source->m_sourceId) return 0;

        KI_INFO_OUT(fmt::format("AUDIO: source={}, sound={}", source->m_sourceId, soundId));

        const auto& [it, _] = m_sources.insert({ source->m_id, std::move(source) });
        return it->first;
    }

    Source* AudioEngine::getSource(
        audio::source_id id)
    {
        const auto& it = m_sources.find(id);
        return it != m_sources.end() ? it->second.get() : nullptr;
    }

    audio::sound_id AudioEngine::registerSound(std::string_view fullPath)
    {
        return m_soundRegistry->registerSound(fullPath);
    }
}
