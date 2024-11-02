#include "AudioEngine.h"

#include <unordered_map>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/util.h"

#include "al_call.h"

#include "model/Node.h"

#include "SoundRegistry.h"
#include "Sound.h"
#include "Source.h"
#include "Listener.h"

#include "engine/UpdateContext.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
}

namespace audio
{
    struct Sound;

    AudioEngine& AudioEngine::get() noexcept
    {
        static AudioEngine s_engine;
        return s_engine;
    }

    AudioEngine::AudioEngine()
        : m_soundRegistry(std::make_unique<SoundRegistry>())
    {
        // NOTE KI null entries to avoid need for "- 1" math
        auto& nullListener = m_listeners.emplace_back<Listener>({});
        auto& nullSource = m_sources.emplace_back<Source>({});

        nullListener.m_matrixLevel = 0;
        nullSource.m_matrixLevel = 0;
    }

    AudioEngine::~AudioEngine()
    {
        for (const auto& source : m_sources) {
            if (!source.m_id) continue;
            source.stop();
        }

        m_listeners.clear();
        m_sources.clear();

        // NOTE KI MUST close buffers before closing context
        m_soundRegistry->clear();

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
    }

    void AudioEngine::update(const UpdateContext& ctx)
    {
        preparePendingListeners(ctx);
        preparePendingSources(ctx);

        auto& nodeRegistry = NodeRegistry::get();

        for (auto& listener : m_listeners) {
            const auto* node = listener.m_nodeHandle.toNode();
            if (!node) continue;

            const auto* snapshot = node->getSnapshotWT();
            if (!snapshot) continue;

            listener.updateFromSnapshot(*snapshot);
        }

        for (auto& source : m_sources) {
            const auto* node = source.m_nodeHandle.toNode();
            if (!node) continue;

            const auto* snapshot = node->getSnapshotWT();
            if (!snapshot) continue;

            source.updateFromSnapshot(*snapshot);
        }
    }

    void AudioEngine::setActiveListener(audio::listener_id id)
    {
        if (m_activeListenerId == id) return;

        m_activeListenerId = id;

        auto* listener = getListener(m_activeListenerId);

        if (!listener) {
            for (const auto& source : m_sources) {
                if (!source.m_id) continue;
                source.pause();
            }
            return;
        }

        listener->update();

        for (const auto& source : m_sources) {
            if (!source.m_id) continue;
            if (!source.isPaused()) continue;
            source.play();
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

    void AudioEngine::toggleSource(audio::source_id id, bool play)
    {
        if (play) {
            playSource(id);
        }
        else {
            stopSource(id);
        }
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
        Listener& listener = m_listeners.emplace_back<Listener>({});
        listener.m_id = static_cast<audio::listener_id>(m_listeners.size() - 1);

        m_pendingListeners.push_back(listener.m_id);

        return listener.m_id;
    }

    Listener* AudioEngine::getListener(
        audio::listener_id id)
    {
        if (id < 1 || id >= m_listeners.size()) return nullptr;
        return &m_listeners[id];
    }

    void AudioEngine::setListenerPos(
        audio::source_id id,
        const glm::vec3& pos,
        const glm::vec3& front,
        const glm::vec3& up)
    {
        if (id < 1 || id >= m_listeners.size()) return;
        auto& listener = m_listeners[id];

        listener.m_pos = pos;
        listener.m_front = front;
        listener.m_up = up;
        listener.updatePos();
    }

    audio::source_id AudioEngine::registerSource(audio::sound_id soundId)
    {
        auto* sound = m_soundRegistry->getSound(soundId);
        if (!sound) return 0;

        sound->prepare();
        if (!sound->m_bufferId) return 0;

        Source& source = m_sources.emplace_back<Source>({});
        source.m_id = static_cast<audio::source_id>(m_sources.size() - 1);

        source.prepare(sound);

        m_pendingSources.push_back(source.m_id);

        KI_INFO_OUT(fmt::format("AUDIO: id={}, source={}, sound={}", source.m_id, source.m_sourceId, soundId));

        return source.m_id;
    }

    Source* AudioEngine::getSource(
        audio::source_id id)
    {
        if (id < 1 || id >= m_sources.size()) return nullptr;
        return &m_sources[id];
    }

    void AudioEngine::setSourcePos(
        audio::source_id id,
        const glm::vec3& pos,
        const glm::vec3& front)
    {
        if (id < 1 || id >= m_sources.size()) return;
        auto& source = m_sources[id];

        source.m_pos = pos;
        source.m_dir = front;
        source.updatePos();
    }

    audio::sound_id AudioEngine::registerSound(std::string_view fullPath)
    {
        return m_soundRegistry->registerSound(fullPath);
    }

    void AudioEngine::preparePendingListeners(const UpdateContext& ctx)
    {
        if (m_pendingListeners.empty()) return;

        auto& nodeRegistry = NodeRegistry::get();

        std::unordered_map<audio::listener_id, bool> prepared;

        for (const auto& id : m_pendingListeners) {
            auto& obj = m_listeners[id];

            const auto* node = obj.m_nodeHandle.toNode();
            if (!node) continue;

            const auto* snapshot = node->getSnapshotWT();
            if (!snapshot) continue;

            obj.updateFromSnapshot(*snapshot);

            if (obj.isReady()) {
                obj.update();
                if (obj.m_default) {
                    setActiveListener(obj.m_id);
                }
                prepared.insert({ id, true });
            }
        }

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_pendingListeners.begin(),
                m_pendingListeners.end(),
                [&prepared](auto& id) {
                    return prepared.find(id) != prepared.end();
                });
            m_pendingListeners.erase(it, m_pendingListeners.end());
        }
    }

    void AudioEngine::preparePendingSources(const UpdateContext& ctx)
    {
        if (m_pendingSources.empty()) return;

        auto& nodeRegistry = NodeRegistry::get();

        std::unordered_map<audio::source_id, bool> prepared;

        for (const auto& id : m_pendingSources) {
            auto& obj = m_sources[id];

            const auto* node = obj.m_nodeHandle.toNode();
            if (!node) continue;

            const auto* snapshot = node->getSnapshotWT();
            if (!snapshot) continue;

            obj.updateFromSnapshot(*snapshot);

            if (obj.isReady()) {
                obj.update();
                if (obj.m_autoPlay) {
                    playSource(obj.m_id);
                }
                prepared.insert({ id, true });
            }
        }

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_pendingSources.begin(),
                m_pendingSources.end(),
                [&prepared](auto& id) {
                    return prepared.find(id) != prepared.end();
                });
            m_pendingSources.erase(it, m_pendingSources.end());
        }
    }
}
