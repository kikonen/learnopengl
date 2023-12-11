#include "AudioEngine.h"

namespace {
}

namespace audio
{
    AudioEngine::AudioEngine(const Assets& assets)
        : m_assets(assets)
    {}

    AudioEngine::~AudioEngine()
    {
    }

    void AudioEngine::prepare()
    {
    }

    void AudioEngine::update(const UpdateContext& ctx)
    {
    }

    void AudioEngine::setActiveListener(audio::listener_id listenerId)
    {
        m_activeListenerId = listenerId;
    }

    void AudioEngine::playSource(audio::source_id)
    {
    }

    void AudioEngine::stopSource(audio::source_id)
    {
    }

    audio::listener_id AudioEngine::registerListener()
    {
        auto listener = std::make_unique<Listener>();
        const auto& [it, _]  = m_listeners.insert({ listener->m_id, std::move(listener) });
        return it->first;
    }

    Listener* AudioEngine::updateListener(
        audio::listener_id listenerId)
    {
        const auto& it = m_listeners.find(listenerId);
        return it != m_listeners.end() ? it->second.get() : nullptr;
    }

    audio::audio_id AudioEngine::registerSource(audio::audio_id audioId)
    {
        auto source = std::make_unique<Source>(audioId);
        const auto& [it, _] = m_sources.insert({ source->m_id, std::move(source) });
        return it->first;
    }

    Source* AudioEngine::updateSource(
        audio::source_id sourceId)
    {
        const auto& it = m_sources.find(sourceId);
        return it != m_sources.end() ? it->second.get() : nullptr;
    }

    audio::audio_id AudioEngine::registerAudio(std::string_view path)
    {
        auto audio = std::make_unique<Audio>(path);
        audio->load(m_assets.assetsDir);

        const auto& [it, _] = m_audios.insert({ audio->m_id, std::move(audio) });
        return it->first;
    }
}

